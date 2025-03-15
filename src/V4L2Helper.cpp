#include "V4L2Helper.h"

std::vector<V4L2Helper::VideoDevice> V4L2Helper::listDevices() {
    std::vector<VideoDevice> devices;
    
#ifdef TARGET_LINUX
    // Linux-specific implementation using V4L2
    ofDirectory dir("/dev");
    dir.allowExt("video*");
    dir.listDir();
    
    for (int i = 0; i < dir.size(); i++) {
        std::string path = dir.getPath(i);
        VideoDevice device;
        device.path = path;
        device.id = i;
        
        // Try to get device name
        int fd = open(path.c_str(), O_RDWR);
        if (fd >= 0) {
            struct v4l2_capability cap;
            if (ioctl(fd, VIDIOC_QUERYCAP, &cap) >= 0) {
                device.name = reinterpret_cast<const char*>(cap.card);
            } else {
                device.name = "Unknown";
            }
            close(fd);
        } else {
            device.name = "Could not open device";
        }
        
        devices.push_back(device);
    }
#else
    // For macOS and other platforms, use ofVideoGrabber
    ofVideoGrabber grabber;
    auto deviceList = grabber.listDevices();
    
    for (int i = 0; i < deviceList.size(); i++) {
        VideoDevice device;
        device.id = deviceList[i].id;
        device.name = deviceList[i].deviceName;
        device.path = "device://" + ofToString(device.id);  // Create a pseudo-path
        devices.push_back(device);
    }
#endif
    return devices;
}

std::vector<V4L2Helper::VideoFormat> V4L2Helper::listFormats(const std::string& devicePath) {
    std::vector<VideoFormat> formats;
    
#ifdef TARGET_LINUX
    // Linux-specific implementation using V4L2
    int fd = open(devicePath.c_str(), O_RDWR);
    if (fd < 0) {
        ofLogError("V4L2Helper") << "Failed to open device: " << devicePath;
        return formats;
    }
    
    struct v4l2_fmtdesc fmtdesc;
    fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmtdesc.index = 0;
    
    while (ioctl(fd, VIDIOC_ENUM_FMT, &fmtdesc) >= 0) {
        VideoFormat format;
        format.pixelFormat = fmtdesc.pixelformat;
        format.name = reinterpret_cast<const char*>(fmtdesc.description);
        format.fourcc = formatCodeToFourCC(fmtdesc.pixelformat);
        
        formats.push_back(format);
        fmtdesc.index++;
    }
    
    close(fd);
#else
    // For macOS and other platforms, provide some standard formats
    // (ofVideoGrabber doesn't let us enumerate formats easily)
    VideoFormat yuyv = {0x56595559, "YUYV 4:2:2", "YUYV"};
    formats.push_back(yuyv);
    
    VideoFormat mjpg = {0x47504A4D, "Motion JPEG", "MJPG"};
    formats.push_back(mjpg);
    
    VideoFormat rgb = {0x33424752, "RGB", "RGB3"};
    formats.push_back(rgb);
#endif
    
    return formats;
}

std::vector<V4L2Helper::Resolution> V4L2Helper::listResolutions(const std::string& devicePath, uint32_t format) {
    std::vector<Resolution> resolutions;
    
#ifdef TARGET_LINUX
    // Linux-specific implementation using V4L2
    int fd = open(devicePath.c_str(), O_RDWR);
    if (fd < 0) {
        ofLogError("V4L2Helper") << "Failed to open device: " << devicePath;
        return resolutions;
    }
    
    // Try to get frame sizes for this format
    struct v4l2_frmsizeenum frmsize;
    frmsize.index = 0;
    frmsize.pixel_format = format;
    
    if (ioctl(fd, VIDIOC_ENUM_FRAMESIZES, &frmsize) < 0) {
        // Device doesn't support enumeration, use common resolutions
        close(fd);
    } else {
        // Device supports enumeration
        if (frmsize.type == V4L2_FRMSIZE_TYPE_DISCRETE) {
            // Discrete sizes
            do {
                Resolution res;
                res.width = frmsize.discrete.width;
                res.height = frmsize.discrete.height;
                resolutions.push_back(res);
                frmsize.index++;
            } while (ioctl(fd, VIDIOC_ENUM_FRAMESIZES, &frmsize) >= 0);
        }
        close(fd);
        
        // If we got resolutions, return them
        if (!resolutions.empty()) {
            return resolutions;
        }
    }
#endif
    
    // For all platforms, or if V4L2 enumeration failed, provide standard resolutions
    Resolution r[] = {
        {640, 480},
        {720, 480},
        {800, 600},
        {1280, 720},
        {1920, 1080}
    };
    
    for (const auto& res : r) {
        resolutions.push_back(res);
    }
    
    return resolutions;
}

bool V4L2Helper::setFormat(const std::string& devicePath, uint32_t format, int width, int height) {
#ifdef TARGET_LINUX
    // Linux-specific implementation using V4L2
    int fd = open(devicePath.c_str(), O_RDWR);
    if (fd < 0) {
        ofLogError("V4L2Helper") << "Failed to open device: " << devicePath;
        return false;
    }
    
    struct v4l2_format fmt;
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    
    // Get the current format first
    if (ioctl(fd, VIDIOC_G_FMT, &fmt) < 0) {
        ofLogError("V4L2Helper") << "Failed to get format for " << devicePath;
        close(fd);
        return false;
    }
    
    // Modify the format
    fmt.fmt.pix.width = width;
    fmt.fmt.pix.height = height;
    fmt.fmt.pix.pixelformat = format;
    
    // Try to set the new format
    if (ioctl(fd, VIDIOC_S_FMT, &fmt) < 0) {
        ofLogError("V4L2Helper") << "Failed to set format "
                                << formatCodeToFourCC(format) << " "
                                << width << "x" << height
                                << " for " << devicePath;
        close(fd);
        return false;
    }
    
    close(fd);
    return true;
#else
    // For macOS, we can't directly set formats, but we'll pretend it worked
    // since we'll use these settings when initializing the camera
    ofLogNotice("V4L2Helper") << "Setting format on non-Linux platform (will apply on camera init): "
                            << width << "x" << height;
    return true;
#endif
}

V4L2Helper::VideoFormat V4L2Helper::getCurrentFormat(const std::string& devicePath) {
    VideoFormat format;
    format.pixelFormat = 0x56595559; // Default to YUYV
    format.name = "Default Format";
    format.fourcc = "DEFT";
    
#ifdef TARGET_LINUX
    // Linux-specific implementation using V4L2
    int fd = open(devicePath.c_str(), O_RDWR);
    if (fd < 0) {
        ofLogError("V4L2Helper") << "Failed to open device: " << devicePath;
        return format;
    }
    
    struct v4l2_format fmt;
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    
    if (ioctl(fd, VIDIOC_G_FMT, &fmt) < 0) {
        ofLogError("V4L2Helper") << "Failed to get format for " << devicePath;
        close(fd);
        return format;
    }
    
    format.pixelFormat = fmt.fmt.pix.pixelformat;
    format.name = formatCodeToName(format.pixelFormat);
    format.fourcc = formatCodeToFourCC(format.pixelFormat);
    
    close(fd);
#endif
    
    return format;
}

uint32_t V4L2Helper::formatNameToCode(const std::string& formatName) {
#ifdef TARGET_LINUX
    // Common format mapping for Linux
    if (formatName == "yuyv422" || formatName == "YUYV 4:2:2" || formatName == "YUYV") {
        return V4L2_PIX_FMT_YUYV;
    } else if (formatName == "rgb565" || formatName == "RGB565") {
        return V4L2_PIX_FMT_RGB565;
    } else if (formatName == "bayer_rggb8" || formatName == "Bayer RGRG/GBGB") {
        return V4L2_PIX_FMT_SRGGB8;
    } else if (formatName == "bayer_bggr8" || formatName == "Bayer BGBG/GRGR") {
        return V4L2_PIX_FMT_SBGGR8;
    } else if (formatName == "bayer_grbg8" || formatName == "Bayer GRGR/BGBG") {
        return V4L2_PIX_FMT_SGRBG8;
    } else if (formatName == "bayer_gbrg8" || formatName == "Bayer GBGB/RGRG") {
        return V4L2_PIX_FMT_SGBRG8;
    } else if (formatName == "yuv411p" || formatName == "Planar YUV 4:1:1") {
        return V4L2_PIX_FMT_YUV411P;
    } else if (formatName == "mjpeg" || formatName == "MJPEG" || formatName == "Motion JPEG" || formatName == "MJPG") {
        return V4L2_PIX_FMT_MJPEG;
    } else if (formatName == "h264" || formatName == "H.264") {
        return V4L2_PIX_FMT_H264;
    }
#endif
    
    // Platform-independent format codes as 4-character codes
    if (formatName == "YUYV" || formatName == "yuyv422" || formatName == "YUYV 4:2:2") {
        return 0x56595559; // YUYV
    } else if (formatName == "MJPG" || formatName == "MJPEG" || formatName == "Motion JPEG") {
        return 0x47504A4D; // MJPG
    } else if (formatName == "RGB3" || formatName == "RGB") {
        return 0x33424752; // RGB3
    }
    
    // Default to YUYV if not recognized
    return 0x56595559; // YUYV
}

std::string V4L2Helper::formatCodeToName(uint32_t pixelFormat) {
#ifdef TARGET_LINUX
    // Linux-specific format names
    switch (pixelFormat) {
        case V4L2_PIX_FMT_YUYV: return "YUYV 4:2:2";
        case V4L2_PIX_FMT_RGB565: return "RGB565";
        case V4L2_PIX_FMT_SRGGB8: return "Bayer RGRG/GBGB";
        case V4L2_PIX_FMT_SBGGR8: return "Bayer BGBG/GRGR";
        case V4L2_PIX_FMT_SGRBG8: return "Bayer GRGR/BGBG";
        case V4L2_PIX_FMT_SGBRG8: return "Bayer GBGB/RGRG";
        case V4L2_PIX_FMT_YUV411P: return "Planar YUV 4:1:1";
        case V4L2_PIX_FMT_MJPEG: return "MJPEG";
        case V4L2_PIX_FMT_H264: return "H.264";
    }
#endif
    
    // Platform-independent format codes
    if (pixelFormat == 0x56595559) return "YUYV 4:2:2"; // YUYV
    if (pixelFormat == 0x47504A4D) return "Motion JPEG"; // MJPG
    if (pixelFormat == 0x33424752) return "RGB"; // RGB3
    
    return "Unknown";
}

std::string V4L2Helper::formatCodeToFourCC(uint32_t pixelFormat) {
    char fourcc[5];
    fourcc[0] = pixelFormat & 0xFF;
    fourcc[1] = (pixelFormat >> 8) & 0xFF;
    fourcc[2] = (pixelFormat >> 16) & 0xFF;
    fourcc[3] = (pixelFormat >> 24) & 0xFF;
    fourcc[4] = 0;
    return std::string(fourcc);
}
