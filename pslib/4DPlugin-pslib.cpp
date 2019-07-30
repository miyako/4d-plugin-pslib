/* --------------------------------------------------------------------------------
 #
 #  4DPlugin-pslib.cpp
 #	source generated by 4D Plugin Wizard
 #	Project : pslib
 #	author : miyako
 #	2019/07/30
 #  
 # --------------------------------------------------------------------------------*/

#include "4DPlugin-pslib.h"

#include <vector>

void CBytes::setBytes(const uint8_t *bytes, unsigned int len)
{
    if(bytes)
    {
        this->_CBytes.resize(len);
        memmove((char *)&this->_CBytes[0], (void *)bytes, len);
    }
}

void CBytes::addBytes(const uint8_t *bytes, unsigned int len)
{
    if(bytes)
    {
        size_t originalSize = this->_CBytes.size();
        this->_CBytes.resize(originalSize + len);
        memmove((char *)&this->_CBytes[originalSize], (void *)bytes, len);
    }
}

const uint8_t *CBytes::getBytesPtr()
{
    if(this->_CBytes.size())
        return (const uint8_t *)&this->_CBytes[0];
    
    return NULL;//    the handle could be NULL if the BLOB is empty on windows
}

uint32_t CBytes::getBytesLength()
{
    return (unsigned int)this->_CBytes.size();
}

CBytes::CBytes()
{
}

CBytes::~CBytes()
{
}

void C_BLOB::setBytes(const uint8_t *bytes, unsigned int len)
{
    this->_CBytes->setBytes(bytes, len);
}

void C_BLOB::addBytes(const uint8_t *bytes, unsigned int len)
{
    this->_CBytes->addBytes(bytes, len);
}

const uint8_t *C_BLOB::getBytesPtr()
{
    return this->_CBytes->getBytesPtr();
}

uint32_t C_BLOB::getBytesLength()
{
    return this->_CBytes->getBytesLength();
}

C_BLOB::C_BLOB() : _CBytes(new CBytes)
{
    
}

C_BLOB::~C_BLOB()
{
    delete _CBytes;
}

void OnStartup() {
    PS_mp_init();
    PS_boot();
}

void OnExit() {
    PS_mp_list_unfreed();
    PS_shutdown();
}

#pragma mark -

void PluginMain(PA_long32 selector, PA_PluginParameters params) {
    
	try
	{
        switch(selector)
        {
            case kInitPlugin :
            case kServerInitPlugin :
                OnStartup();
                break;
                
            case kDeinitPlugin:
                OnExit();
                break;
                
			// --- pslib
            
			case 1 :
				pslib_generate(params);
				break;
                
            case 2 :
                pslib_geometry(params);
                break;

        }

	}
	catch(...)
	{

	}
}

#pragma mark -

size_t writeproc(PSDoc *p, void *data, size_t size)
{
    return size;
}

size_t writeproc2(PSDoc *p, void *data, size_t size)
{
    C_BLOB *buf = (C_BLOB *)PS_get_opaque(p);
    
    if(data)
    {
        buf->addBytes((const uint8_t *)data, (uint32_t)size);
    }
    
    return size;
}

#pragma mark -

#if VERSIONMAC
void convertPathSystemToPosix(std::string& path) {
    
    if(path.length())
    {
        NSString *str = [[NSString alloc]initWithUTF8String:path.c_str()];
        NSURL *u = (NSURL *)CFURLCreateWithFileSystemPath(kCFAllocatorDefault, (CFStringRef)str, kCFURLHFSPathStyle, false);
        [str release];
        CFStringRef p = CFURLCopyFileSystemPath((CFURLRef)u, kCFURLPOSIXPathStyle);
        [u release];
        size_t size = CFStringGetMaximumSizeForEncoding(CFStringGetLength(p), kCFStringEncodingUTF8) + sizeof(uint8_t);
        std::vector<uint8_t> buf(size);
        CFIndex len = 0;
        CFStringGetBytes(p, CFRangeMake(0, CFStringGetLength(p)), kCFStringEncodingUTF8, 0, true, (UInt8 *)&buf[0], size, &len);
        path = std::string((const char *)&buf[0], len);
        CFRelease(p);
    }
    
}
#endif

bool convertToText(const char *encoding, std::string& value) {
    
    /* system iconv recognises utf8; not open source version */
    iconv_t ic = iconv_open("utf-8", encoding);
    
    bool converted = true;
    
    if (ic != (iconv_t)(-1)) {
        char *inData = (char *)value.c_str();
        size_t inDataLen = (size_t)value.length();
        size_t outDataLen = (size_t)(inDataLen * 4) + 4 + 2;
        size_t outDataSize = outDataLen;
        
        char *outData = (char *)calloc(outDataLen, 1);
        char *outDataPtr = outData;
        
        size_t iconv_value;
        
        iconv_value = iconv (ic, &inData, &inDataLen, &outData, &outDataLen);
        
        if (iconv_value)
        {
            converted = false;
        }else{
            value = std::string((const char *)outDataPtr, outDataSize-outDataLen);
        }
        free(outDataPtr);
        iconv_close(ic);
    }
    return converted;
}

bool convertFromText(const char *encoding, std::string& value) {
    
    /* system iconv recognises utf8; not open source version */
    iconv_t ic = iconv_open(encoding, "utf-8");
    
    bool converted = true;
    
    if (ic != (iconv_t)(-1)) {
        char *inData = (char *)value.c_str();
        size_t inDataLen = (size_t)value.length();
        size_t outDataLen = (size_t)(inDataLen * 4) + 4 + 2;
        size_t outDataSize = outDataLen;
        
        char *outData = (char *)calloc(outDataLen, 1);
        char *outDataPtr = outData;
        
        size_t iconv_value;
        
        iconv_value = iconv (ic, &inData, &inDataLen, &outData, &outDataLen);
        
        if (iconv_value)
        {
            converted = false;
        }else{
            value = std::string((const char *)outDataPtr, outDataSize-outDataLen);
        }
        free(outDataPtr);
        iconv_close(ic);
    }
    return converted;
}

#pragma mark -

void setValues(PSDoc *psdoc, PA_ObjectRef o) {
    
    if(o)
    {
        if(ob_get_value_type(o, L"values") == eVK_Object)
        {
            PA_ObjectRef params = ob_get_o(o, L"values");
            if(params)
            {
                CUTF8String json;
                ob_stringify(params, &json);
                
                using namespace std;
                using namespace Json;
                
                Value root;
                CharReaderBuilder builder;
                string errors;
                
                CharReader *reader = builder.newCharReader();
                bool parse = reader->parse((const char *)json.c_str(),
                                           (const char *)json.c_str() + json.size(),
                                           &root,
                                           &errors);
                delete reader;
                
                if(parse)
                {
                    for(Value::const_iterator it = root.begin() ; it != root.end() ; it++)
                    {
                        JSONCPP_STRING name = it.name();
                        
                        if(it->isNumeric())
                        {
                            float value = it->asFloat();
                            PS_set_value(psdoc, name.c_str(), value);
                        }
                    }
                }
                
            }
        }
    }
}

void setParams(PSDoc *psdoc, PA_ObjectRef o) {
    
    if(o)
    {
        if(ob_get_value_type(o, L"params") == eVK_Object)
        {
            PA_ObjectRef params = ob_get_o(o, L"params");
            if(params)
            {
                CUTF8String json;
                ob_stringify(params, &json);
                
                using namespace std;
                using namespace Json;
                
                Value root;
                CharReaderBuilder builder;
                string errors;
                
                CharReader *reader = builder.newCharReader();
                bool parse = reader->parse((const char *)json.c_str(),
                                           (const char *)json.c_str() + json.size(),
                                           &root,
                                           &errors);
                delete reader;
                
                if(parse)
                {
                    for(Value::const_iterator it = root.begin() ; it != root.end() ; it++)
                    {
                        JSONCPP_STRING name = it.name();
                        
                        if(it->isString())
                        {
                            string value = it->asString();
                            if(!name.compare("hyphendict"))
                            {
                                convertPathSystemToPosix(value);
                            }
                            PS_set_parameter(psdoc, name.c_str(), value.c_str());
                        }
                    }
                }
                
            }
        }
    }
}

void getFonts(PSDoc *psdoc, PA_CollectionRef fonts, font_list_t *fontIds) {
    
    if(fonts)
    {
        size_t len = PA_GetCollectionLength(fonts);
        for(PA_long32 i = 0; i < len; ++i)
        {
            PA_Variable v = PA_GetCollectionElement(fonts, i);
            PA_VariableKind k = PA_GetVariableKind(v);
            if(k == eVK_Object)
            {
                PA_ObjectRef o = PA_GetObjectVariable(v);
                std::string font, file;
                ob_get_s(o, L"name", &font);
                ob_get_s(o, L"file", &file);
                
                convertPathSystemToPosix(font);
                convertPathSystemToPosix(file);
                
                int fontId = PS_findfont(psdoc,
                                         font.c_str(),
                                         file.c_str(), (int)ob_get_n(o, L"embed"));
                
                fontIds->push_back(fontId);
            }
        }
    }
}

void processFont(PSDoc *psdoc, PA_ObjectRef d, font_list_t *fontIds) {
    
    if(d)
    {
        if(ob_is_defined(d, L"font"))
        {
            int font = fontIds->at(ob_get_n(d, L"font"));
            double size = ob_get_n(d, L"size");
            PS_setfont(psdoc, font, size);
        }
    }
}

void processText(PSDoc *psdoc, PA_ObjectRef d) {
    
    if(d)
    {
        if(ob_is_defined(d, L"text"))
        {
            std::string value, encoding;
            ob_get_s(d, L"text", &value);
            ob_get_s(d, L"encoding", &encoding);
            
            bool converted = convertFromText((const char *)encoding.c_str(), value);
            
            if(converted)
            {
                setParams(psdoc, d);
                setValues(psdoc, d);

                PS_show_xy2(psdoc,
                            value.c_str(), (int)value.length(),
                            ob_get_n(d, L"x"), ob_get_n(d, L"y"));
            }
        }
    }
}

void processTextArea(PSDoc *psdoc, PA_ObjectRef d) {
    
    if(d)
    {
        if(ob_is_defined(d, L"textArea"))
        {
            std::string value, encoding;
            ob_get_s(d, L"textArea", &value);
            ob_get_s(d, L"encoding", &encoding);
            
            bool converted = convertFromText((const char *)encoding.c_str(), value);
            
            if(converted)
            {
                setParams(psdoc, d);
                setValues(psdoc, d);
                
                std::string mode, feature;
                ob_get_s(d, L"mode", &mode);
                ob_get_s(d, L"feature", &feature);
                PS_show_boxed(psdoc,
                              value.c_str(),
                              ob_get_n(d, L"x"), ob_get_n(d, L"y"),
                              ob_get_n(d, L"width"), ob_get_n(d, L"height"),
                              mode.c_str(),
                              feature.c_str());
            }
        }
    }
}

#pragma mark -

void pslib_geometry(PA_PluginParameters params) {
    
    PSDoc *psdoc = PS_new();
    
    PS_open_mem(psdoc, writeproc);
    
    font_list_t fontIds(0);
    
    PA_CollectionRef returnValue = PA_CreateCollection();
    
    /* load fonts */
    PA_CollectionRef fonts = PA_GetCollectionParameter(params, 1);
    getFonts(psdoc, fonts, &fontIds);
    
    PS_begin_page(psdoc, 0, 0);
    
    PA_CollectionRef texts = PA_GetCollectionParameter(params, 2);
    if(texts)
    {
        size_t len = PA_GetCollectionLength(texts);
        for(PA_long32 i = 0; i < len; ++i)
        {
            PA_Variable v = PA_GetCollectionElement(texts, i);
            PA_VariableKind k = PA_GetVariableKind(v);
            if(k == eVK_Object)
            {
                PA_ObjectRef o = PA_GetObjectVariable(v);
                std::string value, encoding, original;
                ob_get_s(o, L"value", &value);
                ob_get_s(o, L"encoding", &encoding);
                original = value;
                
                int font = fontIds.at(ob_get_n(o, L"font"));
                double size = ob_get_n(o, L"size");
                
                PA_ObjectRef info = PA_CreateObject();
                
                ob_set_s(info, L"encoding", encoding.c_str());
                
                bool converted = convertFromText((const char *)encoding.c_str(), value);
       
                float dimension[3];
                dimension[1] = 0;
                dimension[2] = 0;
                double width = 0;

                if(converted)
                {
                    if(ob_is_defined(o, L"width"))
                    {
                        /* render off screen for textArea */
                        
                        PS_setfont(psdoc, font, size);
                                                
                        std::string mode, feature;
                        ob_get_s(o, L"mode", &mode);
                        ob_get_s(o, L"feature", &feature);
                        
                        setParams(psdoc, o);
                        setValues(psdoc, o);
                        
                        float width = ob_get_n(o, L"width");
                        float height = ob_get_n(o, L"height");
                        
                        size_t unwritten = PS_show_boxed(psdoc,
                                                         value.c_str(),
                                                         0, 0,
                                                         width, height,
                                                         mode.c_str(), feature.c_str());
                        if(unwritten)
                        {
                            std::string text = value.substr(0, value.length() - unwritten);
                            convertToText((const char *)encoding.c_str(), text);
                            ob_set_s(info, L"text", text.c_str());
                            
                            std::string rest = value.substr(value.length() - unwritten, std::string::npos);
                            convertToText((const char *)encoding.c_str(), rest);
                            ob_set_s(info, L"rest", rest.c_str());
                            
                        }else
                        {
                            ob_set_s(info, L"text", original.c_str());
                            ob_set_s(info, L"rest", "");
                        }
 
                        double boxheight = PS_get_value(psdoc, "boxheight", 0.0);
                        
                        ob_set_n(info, L"boxheight", boxheight);
                        ob_set_n(info, L"width", width);
                        ob_set_n(info, L"height", height);
                        
                    }else
                    {
                        /* use PS_string_geometry for text */
                        width = PS_string_geometry(psdoc,
                                                   value.c_str(), (int)value.length(),
                                                   font, size, dimension);
                        
                        ob_set_s(info, L"text", original.c_str());
                        ob_set_n(info, L"width", width);
                        ob_set_n(info, L"desc", dimension[1]);
                        ob_set_n(info, L"asc" , dimension[2]);
                    }

                }
                
                PA_Variable d = PA_CreateVariable(eVK_Object);
                PA_SetObjectVariable(&d, info);
                
                PA_SetCollectionElement(returnValue, i, d);
                
            }
        }
    }
    
    /* cleanup */
    
    for (std::vector<int>::iterator it = fontIds.begin() ; it != fontIds.end(); ++it)
    {
        int fontId = *it;
        PS_deletefont(psdoc, fontId);
    }

    PS_end_page(psdoc);
    
    PS_close(psdoc);
    PS_delete(psdoc);
    
    PA_ReturnCollection(params, returnValue);
}

void pslib_generate(PA_PluginParameters params) {

    C_BLOB buf;
    
    PSDoc *psdoc = PS_new2(NULL, PS_mp_malloc, PS_mp_realloc, PS_mp_free, &buf);
    
    PS_open_mem(psdoc, writeproc2);
    
    std::vector<int> fontIds(0);
    
    PA_ObjectRef command = PA_GetObjectParameter(params, 2);
    if(command) {
        
        std::string keywords,
        subject,
        title,
        creator,
        author,
        boundingBox,
        orientation;
        
        //TODO support unicode meta
        //https://stackoverflow.com/questions/3010015/pdfmark-for-docinfo-metadata-in-pdf-is-not-accepting-accented-characters-in-keyw
        
        /* info before fonts */
        if(ob_get_s(command, L"keywords", &keywords))
        {
            convertFromText("latin1", keywords);
            PS_set_info(psdoc, "Keywords", keywords.c_str());
        }
        if(ob_get_s(command, L"subject", &subject))
        {
            convertFromText("latin1", subject);
            PS_set_info(psdoc, "Subject", subject.c_str());
        }
        if(ob_get_s(command, L"author", &author))
        {
            convertFromText("latin1", author);
            PS_set_info(psdoc, "Author", author.c_str());
        }
        if(ob_get_s(command, L"title", &title))
        {
            convertFromText("latin1", title);
            PS_set_info(psdoc, "Title", title.c_str());
        }
        if(ob_get_s(command, L"creator", &creator))
        {
            convertFromText("latin1", creator);
            PS_set_info(psdoc, "Creator", creator.c_str());
        }
        if(ob_get_s(command, L"boundingBox", &boundingBox))
        {
            PS_set_info(psdoc, "BoundingBox", boundingBox.c_str());
        }
        if(ob_get_s(command, L"orientation", &orientation))
        {
            PS_set_info(psdoc, "Orientation", orientation.c_str());
        }
        
        /* load fonts */
        PA_CollectionRef fonts = PA_GetCollectionParameter(params, 1);
        getFonts(psdoc, fonts, &fontIds);
        
        if(ob_get_value_type(command, L"pages") == eVK_Collection)
        {
            PA_CollectionRef pages = ob_get_c(command, L"pages");
            if(pages)
            {
                /* pages */
                
                for(PA_ulong32 i = 0; i < PA_GetCollectionLength(pages); ++i)
                {
                    PA_Variable v = PA_GetCollectionElement(pages, i);
                    if(PA_GetVariableKind(v) == eVK_Object)
                    {
                        PA_ObjectRef o = PA_GetObjectVariable(v);
                        if(o)
                        {
                            double width = 596;
                            double height = 842;
                            
                            if(ob_is_defined(o, L"width"))
                            {
                                width = ob_get_n(o, L"width");
                            }
                            if(ob_is_defined(o, L"height"))
                            {
                                height = ob_get_n(o, L"height");
                            }
                            
                            PS_begin_page(psdoc, width, height);
                            
                            if(ob_get_value_type(o, L"data") == eVK_Collection)
                            {
                                PA_CollectionRef data = ob_get_c(o, L"data");
                                if(data)
                                {
                                    /* data */
                                    for(PA_ulong32 j = 0; j < PA_GetCollectionLength(data); ++j)
                                    {
                                        PA_Variable datum = PA_GetCollectionElement(data, j);
                                        if(PA_GetVariableKind(datum) == eVK_Object)
                                        {
                                            PA_ObjectRef d = PA_GetObjectVariable(datum);
                                            
                                            processFont(psdoc, d, &fontIds);
                                            processText(psdoc, d);
                                            processTextArea(psdoc, d);
                                            
                                            if(d)
                                            {

                                                
                                                if(ob_is_defined(d, L"draw"))
                                                {
                                                    CUTF8String _draw;
                                                    ob_get_s(d, L"draw", &_draw);
                                                    
                                                    if(!_draw.compare((const uint8_t *)"rect"))
                                                    {
                                                        PS_rect(psdoc,
                                                                ob_get_n(d, L"x"), ob_get_n(d, L"y"),
                                                                ob_get_n(d, L"width"), ob_get_n(d, L"height"));
                                                    }
                                                    if(!_draw.compare((const uint8_t *)"stroke"))
                                                    {
                                                        PS_stroke(psdoc);
                                                    }
   
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                            PS_end_page(psdoc);
                        }
                    }
                }
            }
        }
    }
    
    /* cleanup */
    
    for (std::vector<int>::iterator it = fontIds.begin() ; it != fontIds.end(); ++it)
    {
        int fontId = *it;
        PS_deletefont(psdoc, fontId);
    }
    
    PS_close(psdoc);
    PS_delete(psdoc);
    
    PA_ReturnBlob(params, (void *)buf.getBytesPtr(), (PA_long32)buf.getBytesLength());
}
