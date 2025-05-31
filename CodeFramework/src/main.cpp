#include "Scene.hpp"
#include <chrono>
#include "s1_preprocessing.hpp"
#include "s2_modelling.hpp"
#include "s3_rendering.hpp"
#include "s4_postprocessing.hpp"
#include "Material.hpp"

#define FROM_BIN_TO_PPM    1

int main(int argc, char** argv)
{
    #if !FROM_BIN_TO_PPM

    Scene scene(256, 256);

    if (argc>=2){
        scene.width = static_cast<int>(4.0f / 3.0f * (int)atoi(argv[1]));
        scene.height = (int)atoi(argv[1]);
    }
    else{
        scene.width = static_cast<int>(4.0f / 3.0f * 256);
        scene.height = 256;
    }

    if (argc>=3){
        scene.spp = (int)atoi(argv[2]); 
    }
    else{
        scene.spp = 32;
    }
    // Stage 1

    std::cout << "Resolution: " << scene.width << " x " << scene.height << "\n";

    s1_preprocessing(); // Do Nothing

    std::vector<Material*> materials;
    std::map<std::string, void * > opened_images;

    // Stage 2
    s2_modelling(scene, materials, opened_images);

    auto start = std::chrono::system_clock::now();

    // Stage 3
    s3_rendering(scene);

    auto stop = std::chrono::system_clock::now();

    // Stage 4
    s4_postprocessing(); // Do Nothing


    
    std::cout << "Render complete: \n";
    std::cout << "Time taken: " << std::chrono::duration_cast<std::chrono::hours>(stop - start).count() << " hours\n";
    std::cout << "          : " << std::chrono::duration_cast<std::chrono::minutes>(stop - start).count() << " minutes\n";
    std::cout << "          : " << std::chrono::duration_cast<std::chrono::seconds>(stop - start).count() << " seconds\n";


    for (auto& material : materials) {
        delete material; // Free the memory allocated for each material
    }


    return 0;

    #else

if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <height>\n";
        return 1;
    }
    int height = std::atoi(argv[1]);
    int width = static_cast<int>(4.0f / 3.0f * height);

    size_t num_pixels = static_cast<size_t>(width) * height;
    size_t expected_bytes = num_pixels * sizeof(Vector3f);

    // Scan for fb_dump_*.bin files in current dir
    std::vector<std::filesystem::path> bins;
    for (auto& entry : std::filesystem::directory_iterator(".")) {
        auto p = entry.path();
        if (p.extension() == ".bin" && p.filename().string().rfind("fb_dump_", 0) == 0) {
            bins.push_back(p);
        }
    }
    size_t count = bins.size();
    std::cout << "Found " << count << " .bin files\n";
    if (count == 0) {
        std::cerr << "Error: no fb_dump_*.bin files found.\n";
        return 1;
    }


    
// Accumulate
    std::vector<Vector3f> acc(num_pixels, Vector3f(0.f));

    // ── FIX: Read each bin into a temporary buffer, then add to acc ──
    std::vector<Vector3f> temp(num_pixels);
    for (auto& p : bins) {
        auto sz = std::filesystem::file_size(p);
        if (sz != expected_bytes) {
            std::cerr << "Error: file " << p << " size mismatch: " << sz
                      << " vs " << expected_bytes << "\n";
            return 1;
        }
        std::ifstream fin(p, std::ios::binary);
        if (!fin) {
            std::cerr << "Error opening " << p << "\n";
            return 1;
        }

        // Read into 'temp' buffer, not directly into 'acc'
        fin.read(reinterpret_cast<char*>(temp.data()), expected_bytes);
        if (!fin) {
            std::cerr << "Error reading " << p << "\n";
            return 1;
        }

        // Accumulate pixel‐wise:
        for (size_t i = 0; i < num_pixels; ++i) {
            acc[i] += temp[i];
        }
    }

    // Average:
    for (auto& v : acc) {
        v = v / static_cast<float>(count);
    }




    // Write out PPM
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&t);
    std::ostringstream ss;
    ss << "avg_" << std::put_time(&tm, "%Y%m%d_%H%M%S") << ".ppm";
    std::string out_name = ss.str();

    FILE* fp = std::fopen(out_name.c_str(), "wb");
    if (!fp) {
        std::cerr << "Error: cannot open " << out_name << " for writing\n";
        return 1;
    }
    std::fprintf(fp, "P6\n%d %d\n255\n", width, height);
    for (auto& v : acc) {
        // 有限 SPP 下，高光稀有导致估计偏低；
        // Clamp 截断了极端高样本；
        // Gamma 非线性 又让低 SPP 噪声看起来更暗、更“拉亮”中低值；

        float r = std::clamp(v.x, 0.f, 1.f);
        float g = std::clamp(v.y, 0.f, 1.f);
        float b = std::clamp(v.z, 0.f, 1.f);

        r = std::pow(r, 0.6f);
        g = std::pow(g, 0.6f);
        b = std::pow(b, 0.6f);

        unsigned char uc[3] = {
            static_cast<unsigned char>(255 * r),
            static_cast<unsigned char>(255 * g),
            static_cast<unsigned char>(255 * b)
        };
        std::fwrite(uc, 1, 3, fp);
    }
    std::fclose(fp);
    std::cout << "Wrote averaged image to " << out_name << "\n";
    return 0;

    #endif
}