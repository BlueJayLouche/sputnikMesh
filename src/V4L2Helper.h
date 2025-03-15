#pragma once

#include "ofMain.h"

// Only include V4L2 headers on Linux
#ifdef TARGET_LINUX
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#endif

/**
 * @class V4L2Helper
 * @brief Helper class for configuring video devices (platform-independent)
 */
class V4L2Helper {
public:
    struct VideoFormat {
        uint32_t pixelFormat;  // Pixel format code
        std::string name;      // Human-readable name
        std::string fourcc;    // FourCC code
    };
    
    struct VideoDevice {
        std::string path;      // Device path (e.g., /dev/video0)
        std::string name;      // Device name
        int id;                // Device ID
    };
    
    struct Resolution {
        int width;
        int height;
    };
    
    /**
     * List available video devices
     * @return vector of VideoDevice structures
     */
    static std::vector<VideoDevice> listDevices();
    
    /**
     * List available formats for a device
     * @param devicePath Path to the device (e.g., /dev/video0)
     * @return vector of VideoFormat structures
     */
    static std::vector<VideoFormat> listFormats(const std::string& devicePath);
    
    /**
     * List available resolutions for a device and format
     * @param devicePath Path to the device
     * @param format Pixel format code
     * @return vector of Resolution structures
     */
    static std::vector<Resolution> listResolutions(const std::string& devicePath, uint32_t format);
    
    /**
     * Set the pixel format for a device
     * @param devicePath Path to the device
     * @param format Pixel format code
     * @param width Desired width
     * @param height Desired height
     * @return true if successful
     */
    static bool setFormat(const std::string& devicePath, uint32_t format, int width, int height);
    
    /**
     * Get the current format information for a device
     * @param devicePath Path to the device
     * @return VideoFormat structure with current settings
     */
    static VideoFormat getCurrentFormat(const std::string& devicePath);
    
    /**
     * Convert a format name to a pixel format code
     * @param formatName Name of the format (e.g., "yuyv422")
     * @return Pixel format code
     */
    static uint32_t formatNameToCode(const std::string& formatName);
    
    /**
     * Convert a pixel format code to a format name
     * @param pixelFormat Pixel format code
     * @return Human-readable format name
     */
    static std::string formatCodeToName(uint32_t pixelFormat);
    
    /**
     * Convert a pixel format code to a FourCC string
     * @param pixelFormat Pixel format code
     * @return FourCC string
     */
    static std::string formatCodeToFourCC(uint32_t pixelFormat);
};
