#include <bmp_image.h>
#include <img_lib.h>
#include <jpeg_image.h>
#include <ppm_image.h>

#include <filesystem>
#include <string_view>
#include <iostream>

#include <memory>

using namespace std;

enum class Format {
    BMP,
    JPEG,
    PPM,
    UNKNOWN
};

Format GetFormatByExtension(const img_lib::Path& input_file) {
    const string ext = input_file.extension().string();
    if (ext == ".jpg"sv || ext == ".jpeg"sv) {
        return Format::JPEG;
    }

    if (ext == ".bmp"sv) {
        return Format::BMP;
    }

    if (ext == ".ppm"sv) {
        return Format::PPM;
    }

    return Format::UNKNOWN;
}

class ImageFormatInterface {
public:
    virtual bool SaveImage(const img_lib::Path& file, const img_lib::Image& image) const = 0;
    virtual img_lib::Image LoadImage(const img_lib::Path& file) const = 0;
};

class PPMImageInterface : public ImageFormatInterface {
    bool SaveImage(const img_lib::Path& file, const img_lib::Image& image) const override {
        return img_lib::SavePPM(file, image);
    }
    img_lib::Image LoadImage(const img_lib::Path& file) const override {
        return img_lib::LoadPPM(file);
    }
};

class JPEGImageInterface : public ImageFormatInterface {
    bool SaveImage(const img_lib::Path& file, const img_lib::Image& image) const override {
        return img_lib::SaveJPEG(file, image);
    }
    img_lib::Image LoadImage(const img_lib::Path& file) const override {
        return img_lib::LoadJPEG(file);
    }
};

class BMPImageInterface : public ImageFormatInterface {
    bool SaveImage(const img_lib::Path& file, const img_lib::Image& image) const override {
        return img_lib::SaveBMP(file, image);
    }
    img_lib::Image LoadImage(const img_lib::Path& file) const override {
        return img_lib::LoadBMP(file);
    }
};

ImageFormatInterface* GetFormatInterface(const img_lib::Path& path) {
    const auto format = GetFormatByExtension(path);
    if (format == Format::JPEG) {
        return new JPEGImageInterface();
    } else if  (format == Format::PPM) {
        return new PPMImageInterface();
    } else if (format == Format::BMP) {
        return new BMPImageInterface();
    }
    return nullptr;
}


int main(int argc, const char** argv) {
    if (argc != 3) {
        cerr << "Usage: "sv << argv[0] << " <in_file> <out_file>"sv << endl;
        return 1;
    }

    img_lib::Path in_path = argv[1];
    img_lib::Path out_path = argv[2];

    const unique_ptr<ImageFormatInterface> input_image_interface(GetFormatInterface(in_path));
    if (input_image_interface.get() == nullptr) {
        cerr << "Unknown format of the input file"sv << endl;
        return 2;
    }

    const unique_ptr<ImageFormatInterface> out_image_interface(GetFormatInterface(out_path));
    if (out_image_interface.get() == nullptr) {
        cerr << "Unknown format of the output file"sv << endl;
        return 3;
    }


    img_lib::Image input_image = input_image_interface->LoadImage(in_path);
    if (!input_image) {
        cerr << "Loading failed"sv << endl;
        return 4;
    }

    if (!out_image_interface->SaveImage(out_path, input_image)) {
        cerr << "Saving failed"sv << endl;
        return 5;
    }

    cout << "Successfully converted"sv << endl;
}
