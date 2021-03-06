Shiki Changelog
==================
#### v2.2.7
- Disable iTunes hack on 10.13.5 and newer

#### v2.2.6
- Added Photo Booth to NVIDIA whitelist
- Disabled hardware accelerated renderer hacks on 10.13.4 and newer (Apple fix)
- Removed Sandy Bridge Gen6Accelerator patch due to [better fix](https://github.com/lvs1974/IntelGraphicsFixup/commit/13c64b0659b8eea24189377ff36be35e73474779) in IntelGraphicsFixup 

#### v2.2.5
- Reenable iTunes crashfix hack on 10.13.x (since 10.13.4 seems to only reduce crash frequency)

#### v2.2.4
- Added an AppleGVA patch to fix Sandy Bridge accelerator name (`shikigva=128`)
- Added `/usr/libexec/AirPlayXPCHelper`, ffmpeg, Quick Look, and Photos to the renderer whitelist
- Added Broadwell patches for compatible renderer (discrete GPU hardware acceleration)
- Implemented GPU detection for compatible renderer, whitelist, and snb name (you can remove `shikigva`)
- Fixed invalid Sandy Bridge compatible renderer patches
- Disabled automatic iTunes crashfix hack for 10.13.4 and higher (you may have to reset DRM once you upgrade)

#### v2.2.3
- Fixed a rare crash when IGPU was not renamed
- Fixed a rare crash when patching AppleGVA for Ivy Bridge/AMD compatibility

#### v2.2.2
- Improved compatible renderer patch to support Ivy and Sandy CPUs with AMD/NVIDIA
- Changed `-shikifps` to `shikigva=64`

Note: all the `shikigva` bits are now fully described [here](https://github.com/vit9696/Shiki/blob/master/Shiki/kern_start.cpp#L18).

#### v2.2.1
- Added DaVinci Resolve and mpv to NVIDIA whitelist
- Added FaceTime, iMovie, and Live Screen Capture to NVIDIA whitelist

#### v2.2.0
- Changed iTunes crash patch to a supposedly less destructive workaround
- Added an ability to override mac model in AppleGVA (`shikigva=32`)
- Added an argument to set custom mac model (`shiki-id=<board-id>` defaults to `iMac14,2`)
- Fixed HEVC decoding on 10.13 SKL/KBL & NVIDIA (thx Andrey1970 and all involved)
- Added more processes to the temporary 10.13 NVIDIA whitelist (Chrome, Slack, VS Code)

Note: `#ignore-gpu-blacklist` is necessary to be enabled in `chrome://flags` (see [#13](https://github.com/vit9696/Shiki/issues/13))

#### v2.1.1
- Added a workaround for iTunes crashes in 10.13 (`shikigva=16`)
- Autoenabled iTunes crash workaround in 10.13 with properly configured Ivy+ IGPU

#### v2.1.0
- Requires Lilu 1.2.0 or higher
- Added hardware video decoding fix for SKL/KBL & NVIDIA (`shikigva=4`)
- Added temporary process whitelist for 10.13 SKL/KBL & NVIDIA (`shikigva=12`)

#### v2.0.5
- Added High Sierra installer detection

#### v2.0.4
- Added High Sierra to the list of compatible OS
- Changed `-shikigva` to `shikigva=1`
- Added bit 2 to (e.g. `shikigva=2`) to disable hweBGRA (useful for some AMD)
- Added FAQ in simplified Chinese (thx PMHeart)

#### v2.0.3
- Incorporate safer patches accordingly to calling conventions

#### v2.0.2
- Added OSBundleCompatibleVersion

#### v2.0.1
- Requires Lilu 1.1.0 or newer
- Added more key streaming patches for testing purposes
- Added `-shikigva` boot argument to allow online hardware video decoder

#### v2.0.0
- Rewrote as a [Lilu.kext](https://github.com/vit9696/Lilu) plugin
- Added FAQ entries describing how to workaround Intel Azul freezes
- Opened the source code

#### v1.9.0
- Fixed error logging on 10.12
- Added 10.12.1 beta compatibility

#### v1.8.1
- Added fps.1_0 livestream playback to Safari via `-shikifps` boot argument
- Added version print to the kernel log
- Improved performance on 10.12, should be equal to 10.11 now
- Enabled SSSE3 intrinsics to slightly improve the performance
- Fixed a rare kernel panic on initialisation failure
- Fixed a rare page fault during initialisation
- Fixed page patcher failing to apply some modifications

#### v1.7.0
- Fixed a rare kernel panic on 10.10 and 10.11
- Fixed `-shikifast` mode for 10.12
- Enabled `-shikifast` mode on 10.12 by default

#### v1.5.2
- Disabled Shiki when loading in installer

#### v1.5.1
- Disabled Shiki when loading in recovery

#### v1.5.0
- Added 10.12 Beta support
- Added VMware Fusion support (10.12 only)

#### v1.3.0
- Fixed 10.9.x incompatibilities
- Fixed rare kernel panics and hibernation issues
- Added a possible workaround for 10.10 issues (`-shikislow` boot argument)
- Improved overall stability and performance

#### v1.0.0
- Initial release
