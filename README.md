# YUV_MSE_PSNR
Calculates MSE and PSNR values for the given two .yuv video sequences

THIS PROGRAM CALCULATES MSE AND PSNR VALUES OF TWO .YUV VIDEO SEQUENCES

For windows, use calc_mse_psnr1.exe.
For linux, use calc_mse_psnr

Usage:
Windows: calc_mse_psnr1.exe -r akiyo_cif.yuv -i akiyo_rec0.yuv -size 352x288 -noframes 150
Linux: ./calc_mse_psnr -r akiyo_cif.yuv -i akiyo_rec0.yuv -size 352x288 -noframes 150

1. -r:
This parameter is used to specify the reference yuv video file.
The filename after -r is the reference yuv video.

2. -i:
This parameter is used to specify the input yuv video file whose MSE and PSNR is to be calculated using the reference video.
The filename after -i is the input yuv video filename.

3. -size:
This parameter is used to specify the size of the yuv video files.

4. -noframes:
This parameter specifies the number of frames for which MSE and PSNR is to be calculated.
The number of frames can be less than the actual number of frames contained in the videos.

All the above-mentioned parameters are mandatory for successful running of the compiled binaries.
