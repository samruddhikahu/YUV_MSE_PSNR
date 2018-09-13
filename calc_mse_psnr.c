#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <math.h>

//#ifdef unix
#include <unistd.h>
//#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <stdint.h>

#ifndef O_BINARY
#define O_BINARY 0
#endif

//#ifdef unix
//#define open open
//#define read fread
//#define lseek fseek
//#else
//#define open _open
//#define read _read
//#define lseek _lseeki64
//#endif

//#define tell(fd) lseek(fd, 0, SEEK_CUR)
#define SEEK_SET 0
#define OPENFLAGS_READ  O_RDONLY|O_BINARY
#define MAX_PLANE 3
#define ET_SIZE 300

typedef unsigned char byte;
typedef unsigned short imgpel;
typedef imgpel uint16;
typedef unsigned int uint;
typedef long long int64;
typedef struct size_ {
	int width;
	int height;
}Size;
typedef struct distortion_metric
{
	float value[3];                    //!< current frame distortion
	float average[3];                  //!< average frame distortion
} DistMetric;

char errortext[ET_SIZE];

void size_of_video(char *, int *, int *);
int read_one_frame(int, Size *, byte *, imgpel ***, int);
int readData(int, Size *, byte *);
void buf2img(imgpel **, byte *, int, int);
int get_mem2Dpel(imgpel ***, int, int);
static inline void* mem_malloc(uint);
void no_mem_exit(char *);
void error(char *, int);
int64 compute_SSE(imgpel **, imgpel **, int, int, int, int);
static inline int iabs2(int);
void accumulate_average(DistMetric *, int);
void accumulate_metric(float *, float, int);
static inline float psnr(int, int, float);
	
int main(int argc, char **argv)
{
	//int fh;
	int len1 = strlen(argv[2]);
	char *last_four1 = &argv[2][len1 - 4];

	int len2 = strlen(argv[4]);
	char *last_four2 = &argv[4][len2 - 4];

	int no_frames;
	DistMetric metricSSE;
	DistMetric metricPSNR;

	metricSSE.average[0] = 0.0; metricSSE.average[1] = 0.0; metricSSE.average[2] = 0.0;
	metricPSNR.average[0] = 0.0; metricPSNR.average[1] = 0.0; metricPSNR.average[2] = 0.0;

	if (argc < 9)
	{
		printf("Error: Less number of arguments specified than required !\n");
		exit(0);
	}
	else if (strncmp(argv[1], "-r", 2) != 0)
	{
		printf("Error: Reference video file not specified !\n");
		exit(0);
	}
	else if (strncmp(last_four1, ".yuv", 4) != 0)
	{
		printf("Error: Invalid reference video file format !\n");
		exit(0);
	}
	else if (strncmp(argv[3], "-i", 2) != 0)
	{
		printf("Error: Input video file not specified !\n");
		exit(0);
	}
	else if (strncmp(last_four2, ".yuv", 4) != 0)
	{
		printf("Error: Invalid input video file format !\n");
		exit(0);
	}
	else if (strncmp(argv[5], "-size", 5) != 0)
	{
		printf("Error: Input Video size not specified !\n");
		exit(0);
	}
	else if (strncmp(argv[7], "-noframes", 9) != 0)
	{
		printf("Error: Number of frames not specified !\n");
		exit(0);
	}
	else
	{
		int fh = open(argv[2], OPENFLAGS_READ);
		if (fh == -1)
		{
			printf("Error:Cannot open reference video file !\n");
			exit(0);
		}

		int fi = open(argv[4], OPENFLAGS_READ);
		if (fi == -1)
		{
			printf("Error:Cannot open input video file !\n");
			exit(0);
		}
		else
		{
			printf("Input video file successfully opened.\n");

			//Obtaining video size from command prompt;
			Size *Vidsize = malloc(sizeof(int) * 2);
			//byte *buf;
			size_of_video(argv[6], &Vidsize->width, &Vidsize->height);

			//Obtaining number of frames from command prompt;
			no_frames = atoi(argv[8]);

			//Code for calculation of mse and psnr goes here;
			for (int i = 0; i < no_frames; i++)
			{
				byte *buf_ref = malloc(sizeof(byte)*(Vidsize->height)*(Vidsize->width) * 1.5);
				imgpel **frame_ref[MAX_PLANE];

				int memory_size = get_mem2Dpel(&(frame_ref[0]), Vidsize->height, Vidsize->width);
				memory_size += get_mem2Dpel(&frame_ref[1], (Vidsize->height) / 2, (Vidsize->width) / 2);
				memory_size += get_mem2Dpel(&frame_ref[2], (Vidsize->height) / 2, (Vidsize->width) / 2);

				for (int k = 1; k < 3; k++)
					for (int j = 0; j < (Vidsize->height) / 2; j++)
						for (int i = 0; i < (Vidsize->width) / 2; i++)
							frame_ref[k][j][i] = 128;

				int read_file = read_one_frame(fh, Vidsize, buf_ref, frame_ref, i);

				if (!read_file)
				{
					printf("Error: Cannot read frame !");
					exit(0);
				}

				byte *buf_ip = malloc(sizeof(byte)*(Vidsize->height)*(Vidsize->width) * 1.5);
				imgpel **frame_ip[MAX_PLANE];

				memory_size = get_mem2Dpel(&(frame_ip[0]), Vidsize->height, Vidsize->width);
				memory_size += get_mem2Dpel(&frame_ip[1], (Vidsize->height) / 2, (Vidsize->width) / 2);
				memory_size += get_mem2Dpel(&frame_ip[2], (Vidsize->height) / 2, (Vidsize->width) / 2);

				for (int k = 1; k < 3; k++)
					for (int j = 0; j < (Vidsize->height) / 2; j++)
						for (int i = 0; i < (Vidsize->width) / 2; i++)
							frame_ip[k][j][i] = 128;

				read_file = read_one_frame(fi, Vidsize, buf_ip, frame_ip, i);

				if (!read_file)
				{
					printf("Error: Cannot read frame !");
					exit(0);
				}

				metricSSE.value[0] = (float)compute_SSE(frame_ref[0], frame_ip[0], 0, 0, Vidsize->height, Vidsize->width);
				metricPSNR.value[0] = psnr(65025, (Vidsize->height)*(Vidsize->width), metricSSE.value[0]);

				metricSSE.value[1] = (float)compute_SSE(frame_ref[1], frame_ip[1], 0, 0, (Vidsize->height) / 2, (Vidsize->width) / 2);
				metricPSNR.value[1] = psnr(65025, (Vidsize->height)*(Vidsize->width) / 4, metricSSE.value[1]);

				metricSSE.value[2] = (float)compute_SSE(frame_ref[2], frame_ip[2], 0, 0, (Vidsize->height) / 2, (Vidsize->width) / 2);
				metricPSNR.value[2] = psnr(65025, (Vidsize->height)*(Vidsize->width) / 4, metricSSE.value[2]);

				accumulate_average(&metricSSE, i);
				accumulate_average(&metricPSNR, i);

				/*printf("\n \t \t MSE \t PSNR (dB)\n");
				printf("Y \t %f \t %f \n", metricSSE.value[0]/(Vidsize->width * Vidsize->height), metricPSNR.value[0]);
				printf("U \t %f \t %f \n", metricSSE.value[1]/(Vidsize->width * Vidsize->height / 4), metricPSNR.value[1]);
				printf("V \t %f \t %f \n", metricSSE.value[2]/(Vidsize->width * Vidsize->height / 4), metricPSNR.value[2]);*/
				
			}
			printf("\n \t \t MSE \t PSNR (dB)\n");
			printf("Y \t %f \t %f \n", (metricSSE.average[0])/(Vidsize->width * Vidsize->height), metricPSNR.average[0]);
			printf("U \t %f \t %f \n", (metricSSE.average[1])/(Vidsize->width * Vidsize->height / 4), metricPSNR.average[1]);
			printf("V \t %f \t %f \n", (metricSSE.average[2])/(Vidsize->width * Vidsize->height / 4), metricPSNR.average[2]);
			
		}
	}
	/*printf("\t \t MSE \t PSNR (dB)\n");
	printf("Y \t %f \t %f \n", (metricSSE.value[0])/(Vidsize->width * Vidsize->height), metricPSNR.value[0]);
	printf("U \t %f \t %f \n", metricSSE.value[1]/(Vidsize->width * Vidsize->height / 4), metricPSNR.value[1]);
	printf("V \t %f \t %f \n", metricSSE.value[2]/(Vidsize->width * Vidsize->height / 4), metricPSNR.value[2]);*/

}

void size_of_video(char *buf, int *width, int *height)
{
	char *wd = buf;
	char *ht;
	int x = 0;
	while (*buf != 'x')
	{
		//*wd = *buf;
		buf++;
		x++;
		//wd++;
	}
	//x++;
	buf++;
	ht = buf;
	*(wd + x) = '\0';
	//strncpy(wd, buf1, x);
	*width = atoi(wd);
	*height = atoi(ht);
}

int get_mem2Dpel(imgpel ***array2D, int dim0, int dim1)
{
	int i;

	if ((*array2D = (imgpel**)mem_malloc(dim0 * sizeof(imgpel*))) == NULL)
		no_mem_exit("get_mem2Dpel: array2D");
	if ((*(*array2D) = (imgpel*)mem_malloc(dim0 * dim1 * sizeof(imgpel))) == NULL)
		no_mem_exit("get_mem2Dpel: array2D");

	for (i = 1; i < dim0; i++)
	{
		(*array2D)[i] = (*array2D)[i - 1] + dim1;
	}

	return dim0 * (sizeof(imgpel*) + dim1 * sizeof(imgpel));
}

static inline void* mem_malloc(uint nitems)
{
	void *d;
	if ((d = malloc(nitems)) == NULL)
	{
		no_mem_exit("malloc failed.\n");
		return NULL;
	}
	return d;
}

void no_mem_exit(char *where)
{
	snprintf(errortext, ET_SIZE, "Could not allocate memory: %s", where);
	error(errortext, 100);
}

void error(char *text, int code)
{
	fprintf(stderr, "%s\n", text);
	//flush_dpb(p_Enc->p_Vid->p_Dpb_layer[0], &p_Enc->p_Inp->output);
	//flush_dpb(p_Enc->p_Vid->p_Dpb_layer[1], &p_Enc->p_Inp->output);
	exit(code);
}

int read_one_frame(int vfile, Size *VidSize, byte *buf, imgpel ***imgX, int FrameNo)
{
	//unsigned char *cur_buf = buf;
	const int bytes_y = (VidSize->width)*(VidSize->height);
	const int bytes_uv = (VidSize->width)*(VidSize->height) / 4;

	const int64 framesize_in_bytes = bytes_y + 2 * bytes_uv;

	// Let us seek directly to the current frame
	if (lseek(vfile, framesize_in_bytes * FrameNo, SEEK_SET) == -1)
	{
		snprintf(errortext, ET_SIZE, "read_one_frame: cannot advance file pointer in input file beyond frame %d\n", FrameNo);
		error(errortext, -1);
	}

	//imgpel **imgX = malloc((VidSize->height)*(VidSize->width) * sizeof(imgpel));

	int file_read = readData(vfile, VidSize, buf);

	buf2img(imgX[0], buf, VidSize->height, VidSize->width);
	buf2img(imgX[1], buf + bytes_y, (VidSize->height) / 2, (VidSize->width) / 2);
	buf2img(imgX[2], buf + bytes_y + bytes_uv, (VidSize->height) / 2, (VidSize->width) / 2);

	return file_read;
}

int readData(int vfile, Size *VidSize, byte *buf)
{
	unsigned char *cur_buf = buf;
	int read_size = 1 * VidSize->width;
	for (int i = 0; i < VidSize->height; i++)
	{
		if (read(vfile, cur_buf, read_size) != read_size)
		{
			printf("Unable to read input file !");
			exit(0);
		}
		cur_buf += read_size;
	}

	read_size = 0.5 * VidSize->width;
	for (int j = 0; j < 2; j++)
	{
		for (int i = 0; i < (VidSize->height) / 2; i++)
		{
			if (read(vfile, cur_buf, read_size) != read_size)
			{
				printf("Unable to read input file !");
				exit(0);
			}
			cur_buf += read_size;
		}
	}
	return 1;
}

void buf2img(imgpel **imgX, byte *buf, int ht, int wd)
{
	int j_pos;
	uint16 ui16;

	for (int j = 0; j < ht; j++)
	{
		j_pos = j * wd;
		for (int i = 0; i < wd; i++)
		{
			ui16 = 0;
			memcpy(&(ui16), buf + ((i + j_pos) * 1), 1);
			imgX[j][i] = (imgpel)ui16;
		}
	}
}

int64 compute_SSE(imgpel **imgRef, imgpel **imgSrc, int xRef, int xSrc, int ySize, int xSize)
{
	int i, j;
	imgpel *lineRef, *lineSrc;
	int64 distortion = 0;

	for (j = 0; j < ySize; j++)
	{
		lineRef = &imgRef[j][xRef];
		lineSrc = &imgSrc[j][xSrc];

		for (i = 0; i < xSize; i++)
			distortion += iabs2(*lineRef++ - *lineSrc++);
	}
	return distortion;
}

static inline int iabs2(int x)
{
	return (x) * (x);
}

static inline float psnr(int max_sample_sq, int samples, float sse_distortion)
{
	return (float)(10.0 * log10(max_sample_sq * (double)((double)samples / (sse_distortion < 1.0 ? 1.0 : sse_distortion))));
}

void accumulate_average(DistMetric *metric, int frames)
{
	accumulate_metric(&metric->average[0], metric->value[0], frames);
	accumulate_metric(&metric->average[1], metric->value[1], frames);
	accumulate_metric(&metric->average[2], metric->value[2], frames);
}

void accumulate_metric(float *ave_metric, float cur_metric, int frames)
{
	*ave_metric = (float)(*ave_metric * frames + cur_metric) / (frames + 1);
}
