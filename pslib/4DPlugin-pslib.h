/* --------------------------------------------------------------------------------
 #
 #	4DPlugin-pslib.h
 #	source generated by 4D Plugin Wizard
 #	Project : pslib
 #	author : miyako
 #	2019/07/30
 #  
 # --------------------------------------------------------------------------------*/

#ifndef PLUGIN_PSLIB_H
#define PLUGIN_PSLIB_H

#include "4DPluginAPI.h"

#include "4DPlugin-JSON.h"

#include "iconv/iconv.h"
#include "json/json.h"

#include "pslib.h"
#include "pslib-mp.h"

#pragma mark -

void pslib_generate(PA_PluginParameters params);
void pslib_geometry(PA_PluginParameters params);

bool convertFromText(const char *encoding, std::string& value);
bool convertToText(const char *encoding, std::string& value);

typedef std::vector<int> font_list_t;

#if __APPLE__
#import <CoreFoundation/CoreFoundation.h>
#import <Cocoa/Cocoa.h>
#import <AppKit/AppKit.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif
    
    class CBytes
    {
        
    private:
        
        std::vector<uint8_t> _CBytes;
        
    public:
        
        void setBytes(const uint8_t *bytes, uint32_t len);
        void addBytes(const uint8_t *bytes, uint32_t len);
        
        const uint8_t *getBytesPtr();
        uint32_t getBytesLength();
        
        CBytes();
        ~CBytes();
        
    };
    
    class C_BLOB
    {
        
    private:
        
        CBytes* _CBytes;
        
    public:
        
        void setBytes(const uint8_t *bytes, uint32_t len);
        void addBytes(const uint8_t *bytes, uint32_t len);
        
        const uint8_t *getBytesPtr();
        uint32_t getBytesLength();
        
        C_BLOB();
        ~C_BLOB();
        
    };
    
#ifdef __cplusplus
}
#endif

#endif /* PLUGIN_PSLIB_H */