#include "fileinfo.hpp"

int main(int argc, char **argv)
{
    uint32_t mode = (uint32_t)fileinfo::sortMode::DEFAULT;
    std::string input;
    if (argc == 1)
    {
        fileinfo::lib f;
        // Example of directly getting info with get_path_info.
        fileinfo::statList f1 = f.get_path_info(".");

        std::cout << f1;
        return 0;
    }
    for (int i = 1; i < argc; ++i)
    {
        if(argv[i][0] == '-')
            switch(argv[i][1])
            {
                case 'h':
                    std::cout << "ls with mime info.\n\
options:\n\
-h        print this page.\n\
-s        Sort according to size.\n\
-n        Sort according to name.\n\
-t        Sort according to last access time.\n";
                    return 0;
                case 's':
                    mode = (uint32_t)fileinfo::sortMode::SIZE;
                    break;
                case 'n':
                    mode = (uint32_t)fileinfo::sortMode::NAME;
                    break;
                case 't':
                    mode = (uint32_t)fileinfo::sortMode::ATIME;
                    break;
                default:
                    std::cerr << "Unrecognised option: " << argv[i]
                              << ". Please specify a correct sorting argument.\nUse -h to print a help page.\n";
                    exit(1);

            }
    }

    if(input == "")
        input = ".";

    fileinfo::lib f;
    // Example of using get_sorted_path_info
    fileinfo::statList f1 = f.get_sorted_path_info(input, (fileinfo::sortMode)mode);

    std::cout << f1;

    return 0;
}
