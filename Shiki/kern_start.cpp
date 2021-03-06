//
//  kern_start.cpp
//  Shiki
//
//  Copyright © 2016-2018 vit9696. All rights reserved.
//

#include <Library/LegacyIOService.h>
#include <Headers/plugin_start.hpp>
#include <Headers/kern_api.hpp>
#include <Headers/kern_cpu.hpp>
#include <Headers/kern_file.hpp>
#include <Headers/kern_iokit.hpp>
#include <IOKit/IODeviceTreeSupport.h>

#include "kern_resources.hpp"

// Aside generic DRM unlock patches, which are always on, Shiki also provides a set of patches
// to workaround various issues with hardware video acceleration support.
// These are set as a shikigva boot-arg bitmask.
// For example, to enable ForceOnlineRenderer, ExecutableWhitelist, and ReplaceBoardID
// you sum 1 + 8 + 32 = 41 -> and pass shikigva=41.
enum ShikiGVAPatches {
	// Remove forceOfflineRenderer from AppleGVA Info.plist.
	// This is required to allow hardware acceleration on several mac models with discrete GPUs
	// when only IGPU is available.
	// See /System/Library/PrivateFrameworks/AppleGVA.framework/Resources/Info.plist for mor details.
	ForceOnlineRenderer        = 1,
	// Remve hweBGRA from AppleGVA Info.plist.
	// hweBGRA is not supported by NVIDIA GPUs, so the patch is sometimes required when using NVIDIA
	// in a mac model meant to be used with AMD or Intel.
	// See /System/Library/PrivateFrameworks/AppleGVA.framework/Resources/Info.plist for mor details.
	AllowNonBGRA               = 2,
	// Prior to 10.13.4 certain GPU+CPU combinations were not meant to provide hardware acceleration and had to be patched.
	// The overall problematic configuration list is: NVIDIA+BDW, NVIDIA+SKL, NVIDIA+KBL, AMD+IVB, NVIDIA+SNB.
	// Enabled automatically if shikigva is *NOT* passed on 10.13.3 and earlier. All are fixed in 10.13.4.
	ForceCompatibleRenderer    = 4,
	// Unlike 10.12.6 without security updates and earlier, on 10.13 and latest 10.12.6 AppleGVA patches
	// do not apply to all processes, and each process needs to be patched explicitly. This is a bug
	// in Lilu, which needs to be explored and fixed. For now this bit ensures that the processes present
	// in WHITELIST section of Patches.plist will definitely get the fixes even on 10.13 and 10.12.6.
	// On 10.12.6 and 10.13 this must be used if any of the following bits are used:
	// - ForceOnlineRenderer
	// - AllowNonBGRA
	// - ForceCompatibleRenderer
	// - ReplaceBoardID
	// - FixSandyBridgeClassName
	// It is enabled automatically on 10.12 and 10.13 if shikigva is *NOT* passed and ForceCompatibleRenderer or
	// FixSandyBridgeClassName are automatically enabled.
	AddExecutableWhitelist     = 8,
	// Disable hardware-accelerated FairPlay support in iTunes to fix crashes in 10.13.
	// While this breaks streaming, it is currently the only way to workaround iTunes crashes in 10.13
	// when one has IGPU installed.
	// This is enabled automatically on 10.13.x till 10.13.4 inclusive if shikigva is *NOT* passed via boot-args.
	DisableHardwareKeyExchange = 16,
	// Replace board-id used by AppleGVA by a different board-id.
	// Sometimes it is feasible to use different GPU acceleration settings from the main mac model.
	// By default Mac-27ADBB7B4CEE8E61 (iMac14,2) will be used, but you can override this via shiki-id boot-arg.
	// See /System/Library/PrivateFrameworks/AppleGVA.framework/Resources/Info.plist for mor details.
	ReplaceBoardID             = 32,
	// Attempt to support fps.1_0 (FairPlay 1.0) in Safari.
	// This should technically fix some very old streaming services in Safari, which rely on FairPlay DRM
	// similar to the one found in iTunes. Newer streaming services require FairPlay 2.0, which is hardware-only,
	// so nothing could be done about them.
	// Another way to enable this is to pass -shikifps boot argument.
	UnlockFP10Streaming        = 64,
	// Replace IntelAccelerator with Gen6Accelerator to fix AppleGVA warnings on Sandy Bridge.
	// REMOVED! Implemented more properly in IntelGraphicsFixup:
	// https://github.com/lvs1974/IntelGraphicsFixup/commit/13c64b0659b8eea24189377ff36be35e73474779
	DeprecatedUnused128        = 128
};

static bool autodetectGFX {false};
static char customBoardID[64] {};
static CPUInfo::CpuGeneration cpuGeneration {CPUInfo::CpuGeneration::Unknown};

static void disableSection(uint32_t section) {
	for (size_t i = 0; i < ADDPR(procInfoSize); i++) {
		if (ADDPR(procInfo)[i].section == section)
			ADDPR(procInfo)[i].section = SectionUnused;
	}
	
	for (size_t i = 0; i < ADDPR(binaryModSize); i++) {
		auto patches = ADDPR(binaryMod)[i].patches;
		for (size_t j = 0; j < ADDPR(binaryMod)[i].count; j++) {
			if (patches[j].section == section)
				patches[j].section = SectionUnused;
		}
	}
}

static bool shikiSetCompatibleRendererPatch() {
	// Support 10.10 and higher for now.
	if (getKernelVersion() < KernelVersion::Yosemite)
		return false;

	// Let's look the patch up first.
	UserPatcher::BinaryModPatch *compPatch {nullptr};
	for (size_t i = 0; i < ADDPR(binaryModSize); i++) {
		auto patches = ADDPR(binaryMod)[i].patches;
		for (size_t j = 0; j < ADDPR(binaryMod)[i].count; j++) {
			if (patches[j].section == SectionCOMPATRENDERER) {
				compPatch = &patches[j];
				DBGLOG("shiki", "found compat renderer patch at %lu:%lu with size %lu", i, j, compPatch->size);
				break;
			}
		}
	}

	if (!compPatch)
		return false;

	// I will be frank, the register could change here. But for a good reason it did not for some time.
	// This patch is much simpler than what we had before, so let's stick to it for the time being.

	// lea eax, [rdx-1080000h]
	static uint8_t yosemitePatchFind[] {0x8D, 0x82, 0x00, 0x00, 0xF8, 0xFE};
	static uint8_t yosemitePatchReplace[] {0x8D, 0x82, 0x00, 0x00, 0xF8, 0xFE};
	static constexpr size_t YosemitePatchOff = 2;

	// lea r10d, [r9-1080000h]
	static uint8_t sierraPatchFind[] {0x45, 0x8D, 0x91, 0x00, 0x00, 0xF8, 0xFE};
	static uint8_t sierraPatchReplace[] {0x45, 0x8D, 0x91, 0x00, 0x00, 0xF8, 0xFE};
	static constexpr size_t SierraPatchOff = 3;

	if (getKernelVersion() >= KernelVersion::Sierra) {
		compPatch->find = sierraPatchFind;
		compPatch->replace = sierraPatchReplace;
		compPatch->size = sizeof(sierraPatchFind);
	} else {
		compPatch->find = yosemitePatchFind;
		compPatch->replace = yosemitePatchReplace;
		compPatch->size = sizeof(yosemitePatchFind);
	}

	// Here are all the currently valid IOVARendererID values:
	// 0x1080000 — Sandy Bridge (AppleIntelFramebuffer)
	// 0x1080001 — Sandy Bridge (Gen6Accelerator)
	// 0x1080002 — Ivy Bridge
	// 0x1080004 — Haswell
	// 0x1080008 — Broadwell
	// 0x1080010 — Skylake
	// 0x1080020 — Kaby Lake
	// 0x1020000 — AMD prior to 5xxx (for example, Radeon HD 2600)
	// 0x1020002 — AMD 5xxx and newer
	// 0x1040002 — NVIDIA VP2
	// 0x1040004 — NVIDIA VP3
	// 0x1040008 — NVIDIA VP4 and newer
	// More details are outlined in https://www.applelife.ru/posts/716793.

	// This patch makes AppleGVA believe that we use Haswell, which is not restricted to any modern GPU
	auto generation = CPUInfo::getGeneration();
	if (generation == CPUInfo::CpuGeneration::SandyBridge) {
		*reinterpret_cast<int32_t *>(&yosemitePatchReplace[YosemitePatchOff]) += (0x1080004 - 0x1080001);
		*reinterpret_cast<int32_t *>(&sierraPatchReplace[SierraPatchOff])     += (0x1080004 - 0x1080001);
		return true;
	} else if (generation == CPUInfo::CpuGeneration::IvyBridge) {
		// For whatever reason on GA-Z77-DS3H with i7 3770k and Sapphire Radeon R9 280X attempting to
		// use Haswell-like patch ends in crashes in 10.12.6. Sandy patch works everywhere.
		*reinterpret_cast<int32_t *>(&yosemitePatchReplace[YosemitePatchOff]) += (0x1080000 - 0x1080002);
		*reinterpret_cast<int32_t *>(&sierraPatchReplace[SierraPatchOff])     += (0x1080000 - 0x1080002);
		return true;
	} else if (generation == CPUInfo::CpuGeneration::Broadwell) {
		*reinterpret_cast<int32_t *>(&yosemitePatchReplace[YosemitePatchOff]) += (0x1080004 - 0x1080008);
		*reinterpret_cast<int32_t *>(&sierraPatchReplace[SierraPatchOff])     += (0x1080004 - 0x1080008);
		return true;
	} else if (generation == CPUInfo::CpuGeneration::Skylake) {
		*reinterpret_cast<int32_t *>(&yosemitePatchReplace[YosemitePatchOff]) += (0x1080004 - 0x1080010);
		*reinterpret_cast<int32_t *>(&sierraPatchReplace[SierraPatchOff])     += (0x1080004 - 0x1080010);
		return true;
	} else if (generation == CPUInfo::CpuGeneration::KabyLake) {
		*reinterpret_cast<int32_t *>(&yosemitePatchReplace[YosemitePatchOff]) += (0x1080004 - 0x1080020);
		*reinterpret_cast<int32_t *>(&sierraPatchReplace[SierraPatchOff])     += (0x1080004 - 0x1080020);
		return true;
	}

	return false;
}

static void shikiPatcherLoad(void *, KernelPatcher &) {
	if (autodetectGFX) {
		bool hasExternalAMD = false, hasExternalNVIDIA = false;
		auto sect = WIOKit::findEntryByPrefix("/AppleACPIPlatformExpert", "PCI", gIOServicePlane);
		if (sect) sect = WIOKit::findEntryByPrefix(sect, "AppleACPIPCI", gIOServicePlane);
		if (sect) {
			auto iterator = sect->getChildIterator(gIOServicePlane);
			if (iterator) {
				IORegistryEntry *obj = nullptr;
				while ((obj = OSDynamicCast(IORegistryEntry, iterator->getNextObject())) != nullptr) {
					uint32_t vendor = 0, code = 0;
					if (WIOKit::getOSDataValue(obj, "vendor-id", vendor) &&
						WIOKit::getOSDataValue(obj, "class-code", code) &&
						vendor == WIOKit::VendorID::Intel &&
						code == WIOKit::ClassCode::PCIBridge) {
						DBGLOG("shiki", "found pci bridge %s", safeString(obj->getName()));
						auto gpuiterator = IORegistryIterator::iterateOver(obj, gIOServicePlane, kIORegistryIterateRecursively);
						if (gpuiterator) {
							IORegistryEntry *gpuobj = nullptr;
							while ((gpuobj = OSDynamicCast(IORegistryEntry, gpuiterator->getNextObject())) != nullptr) {
								uint32_t gpuvendor = 0, gpucode = 0;
								DBGLOG("shiki", "found %s on pci bridge", safeString(gpuobj->getName()));
								if (WIOKit::getOSDataValue(gpuobj, "vendor-id", gpuvendor) &&
									WIOKit::getOSDataValue(gpuobj, "class-code", gpucode) &&
									(gpucode == WIOKit::ClassCode::DisplayController || gpucode == WIOKit::ClassCode::VGAController)) {
									if (gpuvendor == WIOKit::VendorID::ATIAMD) {
										DBGLOG("shiki", "found AMD GPU device %s", safeString(gpuobj->getName()));
										hasExternalAMD = true;
										break;
									} else if (gpuvendor == WIOKit::VendorID::NVIDIA) {
										DBGLOG("shiki", "found NVIDIA GPU device %s", safeString(gpuobj->getName()));
										hasExternalNVIDIA = true;
										break;
									}
								}
							}

							gpuiterator->release();
						}
					}
				}
				iterator->release();
			}
		}

		bool disableWhitelist = cpuGeneration != CPUInfo::CpuGeneration::SandyBridge;
		bool disableCompatRenderer = true;
		if (hasExternalNVIDIA && (cpuGeneration == CPUInfo::CpuGeneration::SandyBridge ||
			cpuGeneration == CPUInfo::CpuGeneration::Broadwell ||
			cpuGeneration == CPUInfo::CpuGeneration::Skylake ||
			cpuGeneration == CPUInfo::CpuGeneration::KabyLake)) {
			disableCompatRenderer = false;
			disableWhitelist = false;
		} else if (hasExternalAMD && cpuGeneration == CPUInfo::CpuGeneration::IvyBridge) {
			disableCompatRenderer = false;
			disableWhitelist = false;
		}

		if (disableCompatRenderer)
			disableSection(SectionCOMPATRENDERER);

		if (disableWhitelist)
			disableSection(SectionWHITELIST);

		DBGLOG("shiki", "autodetedect decision: whitelist %d compat %d for cpu %d nvidia %d amd %d",
			   !disableWhitelist, !disableCompatRenderer, cpuGeneration, hasExternalNVIDIA, hasExternalAMD);
	}

	if (customBoardID[0]) {
		auto entry = IORegistryEntry::fromPath("/", gIODTPlane);
		if (entry) {
			DBGLOG("shiki", "changing shiki-id to %s", customBoardID);
			entry->setProperty("shiki-id", OSData::withBytes(customBoardID, static_cast<uint32_t>(strlen(customBoardID)+1)));
			entry->release();
		} else {
			SYSLOG("shiki", "failed to obtain iodt tree");
		}
	}
}

static void shikiStart() {
	bool forceOnlineRenderer     = false;
	bool allowNonBGRA            = false;
	bool forceCompatibleRenderer = false;
	bool addExecutableWhitelist  = false;
	bool disableKeyExchange      = false;
	bool replaceBoardID          = false;
	bool unlockFP10Streaming     = false;

	cpuGeneration = CPUInfo::getGeneration();

	int bootarg {0};
	if (PE_parse_boot_argn("shikigva", &bootarg, sizeof(bootarg))) {
		forceOnlineRenderer     = bootarg & ForceOnlineRenderer;
		allowNonBGRA            = bootarg & AllowNonBGRA;
		forceCompatibleRenderer = bootarg & ForceCompatibleRenderer;
		addExecutableWhitelist  = bootarg & AddExecutableWhitelist;
		disableKeyExchange      = bootarg & DisableHardwareKeyExchange;
		replaceBoardID          = bootarg & ReplaceBoardID;
		unlockFP10Streaming     = bootarg & UnlockFP10Streaming;
	} else {
		// By default enable iTunes hack on 10.13-10.13.4 for Sandy+ IGPUs
		disableKeyExchange = getKernelVersion() == KernelVersion::HighSierra && getKernelMinorVersion() <= 5 &&
		(cpuGeneration == CPUInfo::CpuGeneration::SandyBridge || cpuGeneration == CPUInfo::CpuGeneration::IvyBridge ||
		 cpuGeneration == CPUInfo::CpuGeneration::Haswell || cpuGeneration == CPUInfo::CpuGeneration::Broadwell ||
		 cpuGeneration == CPUInfo::CpuGeneration::Skylake || cpuGeneration == CPUInfo::CpuGeneration::KabyLake);

		if (PE_parse_boot_argn("-shikigva", &bootarg, sizeof(bootarg))) {
			SYSLOG("shiki", "-shikigva is deprecated use shikigva %d bit instead", ForceOnlineRenderer);
			forceOnlineRenderer = true;
		}

		// Starting with 10.13.4 Apple has fixed AppleGVA to no longer require patching for compatible renderer.
		if ((getKernelVersion() == KernelVersion::HighSierra && getKernelMinorVersion() < 5) ||
			getKernelVersion() < KernelVersion::HighSierra) {
			autodetectGFX = cpuGeneration == CPUInfo::CpuGeneration::SandyBridge ||
				cpuGeneration == CPUInfo::CpuGeneration::IvyBridge ||
				cpuGeneration == CPUInfo::CpuGeneration::Broadwell ||
				cpuGeneration == CPUInfo::CpuGeneration::Skylake ||
				cpuGeneration == CPUInfo::CpuGeneration::KabyLake;

			if (autodetectGFX) {
				forceCompatibleRenderer = true;
				addExecutableWhitelist = getKernelVersion() >= KernelVersion::Sierra;
			}
		}

		DBGLOG("shiki", "will autodetect autodetect GFX %d whitelist %d", autodetectGFX, addExecutableWhitelist);
	}

	if (PE_parse_boot_argn("-shikifps", &bootarg, sizeof(bootarg))) {
		SYSLOG("shiki", "-shikifps is deprecated use shikigva %d bit instead", UnlockFP10Streaming);
		unlockFP10Streaming = true;
	}

	DBGLOG("shiki", "pre-config: online %d, bgra %d, compat %d, whitelist %d, ke1 %d, id %d, stream %d",
		forceOnlineRenderer, allowNonBGRA, forceCompatibleRenderer, addExecutableWhitelist, disableKeyExchange, replaceBoardID,
		unlockFP10Streaming);

	// Disable unused sections
	if (!forceOnlineRenderer)
		disableSection(SectionOFFLINE);

	if (!allowNonBGRA)
		disableSection(SectionBGRA);

	// Compatible renderer patch varies depending on the CPU.
	if (!forceCompatibleRenderer || !shikiSetCompatibleRendererPatch())
		disableSection(SectionCOMPATRENDERER);

	if (!addExecutableWhitelist)
		disableSection(SectionWHITELIST);
	
	if (!disableKeyExchange)
		disableSection(SectionKEGVA);

	// Custom board-id may be overridden by a boot-arg
	if (replaceBoardID) {
		if (!PE_parse_boot_argn("shiki-id", customBoardID, sizeof(customBoardID)))
			snprintf(customBoardID, sizeof(customBoardID), "Mac-27ADBB7B4CEE8E61"); // iMac14,2
		DBGLOG("shiki", "requesting %s board-id for gva", customBoardID);
	} else {
		disableSection(SectionBOARDID);
	}

	if (!unlockFP10Streaming)
		disableSection(SectionNSTREAM);

	// Schedule onPatcherLoad if we need to autodetect IGPU or to set a custom board-id
	if (autodetectGFX || replaceBoardID) {
		auto err = lilu.onPatcherLoad(shikiPatcherLoad);
		if (err != LiluAPI::Error::NoError)
			SYSLOG("shiki", "unable to attach to patcher load %d", err);
	}

	auto err = lilu.onProcLoad(ADDPR(procInfo), ADDPR(procInfoSize), nullptr, nullptr, ADDPR(binaryMod), ADDPR(binaryModSize));
	if (err != LiluAPI::Error::NoError)
		SYSLOG("shiki", "unable to attach to process load %d", err);
}

static const char *bootargOff[] {
	"-shikioff"
};

static const char *bootargDebug[] {
	"-shikidbg"
};

static const char *bootargBeta[] {
	"-shikibeta"
};

PluginConfiguration ADDPR(config) {
	xStringify(PRODUCT_NAME),
	parseModuleVersion(xStringify(MODULE_VERSION)),
	LiluAPI::AllowNormal,
	bootargOff,
	arrsize(bootargOff),
	bootargDebug,
	arrsize(bootargDebug),
	bootargBeta,
	arrsize(bootargBeta),
	KernelVersion::Mavericks,
	KernelVersion::HighSierra,
	shikiStart
};
