
#include <string.h>
#include <math.h>
#include <png.h>

void plot_point(unsigned char* imageb, const unsigned &w, const unsigned &x, const unsigned &y)
{
    imageb[y*w + x] = 255;
}

void line(int x0, int y0, int x1, int y1, unsigned char* b, const int &w)
{
    int dx = abs(x1-x0), sx = x0<x1 ? 1 : -1;
    int dy = abs(y1-y0), sy = y0<y1 ? 1 : -1; 
    int err = (dx>dy ? dx : -dy)/2, e2;

    for(;;){
        plot_point(b, w, x0,y0);
        if (x0==x1 && y0==y1) break;
        e2 = err;
        if (e2 >-dx) { err -= dy; x0 += sx; }
        if (e2 < dy) { err += dx; y0 += sy; }
    }
}

void draw_hogfv(unsigned char* buffer, const int &w, const float (&hogfv)[6156])
{
    float line_length = 6;

    float angl_per_bin = M_PI / 9;

    unsigned blocks_per_col = 19;
    unsigned blocks_per_row = 9;
    unsigned nr_blocks = blocks_per_col*blocks_per_row;
    unsigned histograms_per_block = 4;
    unsigned bins_per_histogram = 9;

    for(unsigned by=0;by<blocks_per_col;++by){    
        for(unsigned bx=0;bx<blocks_per_row;++bx){
            unsigned bi = by*blocks_per_row + bx;
            int x_offset = 10+bx*10;
            int y_offset = 10+by*10;
            for(unsigned hi=0;hi<histograms_per_block;++hi){
                for(unsigned bin_i=0;bin_i<bins_per_histogram;++bin_i){
                    unsigned i = bi*histograms_per_block*bins_per_histogram + hi*bins_per_histogram + bin_i;

                    float angle = bin_i*angl_per_bin;
                    int x0 = x_offset - cos(angle)*line_length*hogfv[i];
                    int y0 = y_offset - sin(angle)*line_length*hogfv[i];
                    int x1 = x_offset + cos(angle)*line_length*hogfv[i];
                    int y1 = y_offset + sin(angle)*line_length*hogfv[i];

                    line(x0, y0, x1, y1, buffer, w);
                }
            }
        }
    }
}

void make_histograms(float (&his)[1800],const float (&dir)[20000],const float (&mag)[20000],const unsigned int &width, const unsigned int &height)
{
    unsigned nr_bins = 9;
    unsigned angle_per_bin = 20;
    unsigned cells_ver = 20;
    unsigned cells_hor = 10;
    unsigned cs = 10;

    unsigned cell_index = 0;
    for(unsigned cy=0;cy<cells_ver;++cy){
        for(unsigned cx=0;cx<cells_hor;++cx){
            for(unsigned y=0;y<cs;++y){
                for(unsigned x=0;x<cs;++x){

                    unsigned i = width*( cy*cs + y ) + cx*cs + x;

                    float fpi = dir[i] / angle_per_bin;
                    float ratio = fmod(fpi, 1);

                    unsigned int bin_nr = fpi - ratio;
                    bin_nr = bin_nr % nr_bins;
                    
                    his[cell_index*nr_bins + bin_nr] += (1 - ratio)*mag[i];
                    his[cell_index*nr_bins + bin_nr+1] += ratio*mag[i];
                }
            }
            ++cell_index;
        }
    }
}

void calc_dir(float (&dir)[20000], float (&mag)[20000], const float (&gx)[20000], const float (&gy)[20000], const unsigned int &width, const unsigned int &height)
{
    const double eps=1.e-50;
    for(unsigned y=0;y<height;++y){
        for(unsigned x=0;x<width;++x){
            unsigned i = x+y*width;
            dir[i] = 90 + 180*atan(gy[i] / (gx[i]+eps))/M_PI;
            mag[i] = sqrt( gx[i]*gx[i] + gy[i]*gy[i] );
        }
    }
}

void calc_gy(float (&gy)[20000], unsigned char* buffer, const unsigned int &width, const unsigned int &height)
{
    for(unsigned x=0;x<width;++x){
        for(unsigned y=1, ymax=height-1;y<ymax;++y){
            unsigned i = x + y*width;
            gy[i] = buffer[i+width] - buffer[i-width];
        }
    }
}

void calc_gx(float (&gx)[20000], unsigned char* buffer, const unsigned int &width, const unsigned int &height)
{
    for(unsigned y=0;y<height;++y){
        for(unsigned x=1, xmax=width-1;x<xmax;++x){
            unsigned i = x + y*width;
            gx[i] = buffer[i+1] - buffer[i-1];
        }
    }
}

void make_hogfv(float (&hist)[1800], float (&vec)[6156])
{
    unsigned bins_per_cell = 9;
    unsigned cells_per_row = 10;
    unsigned bins_per_row = bins_per_cell*cells_per_row;

    unsigned blocks_per_row = 9;
    unsigned blocks_per_col = 19;
    
    unsigned vi = 0;
    // Move to next block
    for(unsigned cy=0;cy<blocks_per_col;++cy){
        for(unsigned cx=0;cx<blocks_per_row;++cx){
            unsigned base_index = cy*bins_per_row + cx*bins_per_cell;
            // Traverse block
            unsigned normalizer = 0;
            for(unsigned y=0;y<2;++y){
                for(unsigned x=0;x<2*bins_per_cell;++x){
                    unsigned hi = base_index + y*bins_per_row + x;
                    normalizer += hist[hi] * hist[hi];
                }
            }

            normalizer = sqrt(normalizer);

            for(unsigned y=0;y<2;++y){
                for(unsigned x=0;x<2*bins_per_cell;++x, ++vi){
                    unsigned hi = base_index + y*bins_per_row + x;
                    vec[vi] = hist[hi] / normalizer;
                }
            }
        }
    }
}

int main()
{
    png_image img;
    memset(&img, 0, (sizeof img));
    img.version = PNG_IMAGE_VERSION;

    png_image out;
    memset(&out, 0, sizeof(out));
    out.version = PNG_IMAGE_VERSION;
    
    if(png_image_begin_read_from_file(&img, "images/pedestrian.png"))
    {
        img.format = PNG_FORMAT_GRAY;
        
        png_bytep buffer;
        size_t buffer_size = PNG_IMAGE_SIZE( img );
        buffer = (unsigned char*)malloc( buffer_size );

        out.format = PNG_FORMAT_GRAY;
        out.format = img.format;
        out.width = img.width;
        out.height = img.height;
        out.flags = 0;
        // out.colormap_entries = 256;
        out.opaque = NULL;
        out.warning_or_error = 0;

        png_bytep out_buffer;
        size_t out_buffer_size PNG_IMAGE_SIZE( out );
        out_buffer = (unsigned char*)malloc( out_buffer_size );

        float gx[20000];
        memset(gx,0,sizeof(gx));

        float gy[20000];
        memset(gy,0,sizeof(gy));

        float dir[20000];
        memset(dir,0,sizeof(dir));

        float mag[20000];
        memset(mag,0,sizeof(mag));

        float his[1800];
        memset(his,0,sizeof(his));

        // HOG Feature Vector, 36 x 9 x 19
        float vec[6156];
        memset(vec,0,sizeof(vec));

        if(buffer != NULL && png_image_finish_read( &img, NULL, buffer, 0, NULL )){

            calc_gx(gx,buffer,img.width,img.height);
            calc_gy(gy,buffer,img.width,img.height);
            calc_dir(dir,mag,gx,gy,img.width,img.height);
            make_histograms(his,dir,mag,img.width,img.height);
            make_hogfv(his, vec);
            draw_hogfv(out_buffer, img.width, vec);

            bool success = true;
            //success = success && png_image_write_to_file( &img, "images/orig.png", 0, buffer, 0, NULL);
            success = success && png_image_write_to_file( &out, "images/out.png", 0, out_buffer, 0, NULL);

            if(success){
                exit(EXIT_SUCCESS);
            }

        }else{

            if( buffer == NULL ){
                png_image_free( &img );
            }else{
                free( buffer );
            }

            if( buffer == NULL ){
                png_image_free( &out );
            }else{
                free( out_buffer );
            }
        }
    }

    fprintf(stderr, "error:%s\n", img.message);
    fprintf(stderr, "error:%s\n", out.message);
    exit(EXIT_FAILURE);
}
