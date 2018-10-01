
// g++ -o main main.o -lpng16

#include <iostream>
#include <png.h>

int main()
{
    if(PNG_LIBPNG_VER < 10620)
    {
        std::cout << "\nThis executable preferably uses pnglib version 1.6.20 or greater." << std::endl;
    }

    FILE * fp = fopen("images/pedestrians_walking.png", "rb");
    
    if(!fp)
    {
        std::cout << "\nFailed to open file." << std::endl;
        return 0;
    }

    size_t sig_size = 8;
    unsigned char sig[sig_size];
    fread(sig, 1, sig_size, fp);

    if(png_sig_cmp( sig, 0, sig_size ))
    {
        std::cout << "\nSignature does not match PNG." << std::endl;
        fclose(fp);
        return 0;
    }

    

    fclose(fp);

    return 0;
}
