//                                                   
//  kern_resources.hpp                               
//  Shiki                                            
//                                                   
//  Copyright © 2016-2017 vit9696. All rights reserved.   
//                                                   
//  This is an autogenerated file!                   
//  Please avoid any modifications!                  
//                                                   

#include <Headers/kern_user.hpp>                     
#include <stdint.h>                                  

extern UserPatcher::BinaryModInfo ADDPR(binaryMod)[];
extern const size_t ADDPR(binaryModSize);            

extern UserPatcher::ProcInfo ADDPR(procInfo)[];      
extern const size_t ADDPR(procInfoSize);             


// Section list

enum : uint32_t {
	SectionUnused = 0,
	SectionOFFLINE = 1,
	SectionFCPUID = 2,
	SectionNDRMI = 3,
	SectionBGRA = 4,
	SectionBOARDID = 5,
	SectionNSTREAM = 6,
	SectionCOMPATRENDERER = 7,
	SectionKEGVA = 8,
	SectionWHITELIST = 9,
};
