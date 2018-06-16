#include <cstdio>
#include <iostream>
#include <unistd.h>
#include <cell_world.hpp>
#include <life.hpp>
#include <util.hpp>

static void generate(int width, int height, int percent, 
    const std::string& filename)
{
    cell_world gol = cell_world::create_random(width, height, percent);
    gol.save_grid(filename);
}

static void benchmark(int width, int height, int percent, int gens)
{
    cell_world gol_seq = cell_world::create_random(width, height, percent, 
                         life_cpu_seq);
    cell_world gol_simd = cell_world::create_from_existing(gol_seq);
    gol_simd.lifesim = life_cpu_simd;
    cell_world gol_omp = cell_world::create_from_existing(gol_seq);
    gol_omp.lifesim = life_cpu_omp;
    cell_world gol_gpu = cell_world::create_from_existing(gol_seq);
    gol_gpu.lifesim = life_gpu_ocl;

    double gol_seq_time = gol_seq.simulate(gens);
    double gol_simd_time = gol_simd.simulate(gens);
    double gol_omp_time = gol_omp.simulate(gens);
    double gol_gpu_time = gol_gpu.simulate(gens);

    printf("+------------------------------+\n");
    printf("| Implementation | Runtime (s) |\n");
    printf("|------------------------------|\n");
    printf("| CPU Sequential | %11.2f |\n", gol_seq_time);
    printf("| CPU SIMD 1T    | %11.2f |\n", gol_simd_time);
    printf("| CPU OpenMP     | %11.2f |\n", gol_omp_time);
    printf("| GPU OpenCL     | %11.2f |\n", gol_gpu_time);
    printf("+------------------------------+\n\n");

    if (gol_seq == gol_simd && gol_seq == gol_omp && gol_seq == gol_gpu) {
        std::cout << "All implementations are equal" << std::endl;
        return;
    }
    if (gol_seq != gol_simd) {
        std::cout << "CPU SIMD is not equal to the reference implementation"
                  << std::endl;
    }
    if (gol_seq != gol_omp) {
        std::cout << "CPU OpenMP is not equal to the reference implementation"
                  << std::endl;
    }
    if (gol_seq != gol_gpu) {
        std::cout << "GPU OpenCL is not equal to the reference implementation"
                  << std::endl;
    }
}

int main(int argc, char** argv)
{
    benchmark(1024, 1024, 50, 1000);
    /*
    char flag;

    char mode = -1;
    int width = -1;
    int height = -1;
    int percent = 50;
    int gens = -1;
    int sim = -1;
    std::string infile = "";
    std::string outfile = "";

    while ((flag = getopt(argc, argv, ":brs:g:h:i:o:p:w:")) != -1) {
        switch (flag) {
            case 'b':
            case 'r':
                mode = flag;
                break;
            
            case 's':
                mode = flag;
                sim = atoi(optarg);
                break;
            
            case 'g':
                gens = atoi(optarg);
                break;
            
            case 'h':
                height = atoi(optarg);
                break;
            
            case 'i':
                infile = optarg;
                break;
            
            case 'o':
                outfile = optarg;
                break;
            
            case 'p':
                percent = atoi(optarg);
                break;
            
            case 'w':
                width = atoi(optarg);
                break;

            case ':':
                fprintf(stderr, "%s: '-%c' requires an argument\n", argv[0], 
                    optopt);
                exit(1);
            
            case '?':
                fprintf(stderr, "%s: invalid option '-%c'\n", argv[0], optopt);
                exit(1);
            
            default:
                break;
        }
    }*/
    return 0;
}
