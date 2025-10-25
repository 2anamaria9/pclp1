#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define NMAX 100
typedef struct {
	int R, G, B, p;
	int x1, y1, x2, y2;
} image;

void free_pixels(image ***pixels, int height)
{
	//se elibereaza memoria
	if (*pixels) {
		for (int i = 0; i < height; i++) {
			free((*pixels)[i]);
		}
		free(*pixels);
		*pixels = NULL;
	}
}

void load(char *name, char *type, int *w, int *h, int *val, image ***pixels)
{
	//in caz ca exista deja o imagine incarcata, se actualizeaza
	if (*pixels) {
		free_pixels(pixels, *h);
	}
	FILE *file = fopen(name, "r");
	if (!file) {
		printf("Failed to load %s\n", name);
		return;
	} //se deschide fisierul si se citesc datele
	if (fscanf(file, "%2s", type) != 1 ||
		(strcmp(type, "P5") != 0 && strcmp(type, "P6") != 0 &&
		strcmp(type, "P2") != 0 && strcmp(type, "P3") != 0)) {
		printf("Failed to load %s\n", name);
		fclose(file);
		return;
	} //tipul fisierului
	char buffer[256];
	while (fgets(buffer, sizeof(buffer), file)) {
		if (buffer[0] != '#')
			break;
	} //se ignora liniile de comentarii
	if (fscanf(file, "%d %d", w, h) != 2) {
		printf("Failed to load %s\n", name);
		fclose(file);
		return;
	} //dimensiunile imaginii
	if (fscanf(file, "%d", val) != 1 || *val > 255) {
		printf("Failed to load %s\n", name);
		fclose(file);
		return;
	} //valoarea maxima a pixelilor
	fgetc(file);
	*pixels = (image **)malloc(*h * sizeof(image *));
	if (!*pixels) {
		fprintf(stderr, "malloc failed\n");
		fclose(file);
		return;
	}
	for (int i = 0; i < *h; i++) {
		(*pixels)[i] = (image *)malloc(*w * sizeof(image));
		if (!(*pixels)[i]) {
			fprintf(stderr, "malloc failed\n");
			fclose(file);
			return;
		}
	}
	if (strcmp(type, "P2") == 0) {
		for (int i = 0; i < *h; i++) {
			for (int j = 0; j < *w; j++) {
				fscanf(file, "%d", &(*pixels)[i][j].p);
			}
		} //imagine alb-negru, format text
	} else if (strcmp(type, "P3") == 0) {
		for (int i = 0; i < *h; i++) {
			for (int j = 0; j < *w; j++) {
				fscanf(file, "%d", &(*pixels)[i][j].R);
				fscanf(file, "%d", &(*pixels)[i][j].G);
				fscanf(file, "%d", &(*pixels)[i][j].B);
			}
		} //imagine color, format text
	} else if (strcmp(type, "P5") == 0) {
		for (int i = 0; i < *h; i++) {
			for (int j = 0; j < *w; j++) {
				(*pixels)[i][j].p = fgetc(file);
			}
		} //imagine alb-negru, format binar
	} else if (strcmp(type, "P6") == 0) {
		for (int i = 0; i < *h; i++) {
			for (int j = 0; j < *w; j++) {
				(*pixels)[i][j].R = fgetc(file);
				(*pixels)[i][j].G = fgetc(file);
				(*pixels)[i][j].B = fgetc(file);
			}
		} //imagine color, format binar
	}
	printf("Loaded %s\n", name);
	fclose(file);
}

void selection(int x1, int y1, int x2, int y2, int w, int h, image *s)
{
	//se asigura ordinea crescatoare a coordonatelor
	if (x1 > x2) {
		int temp = x1;
		x1 = x2;
		x2 = temp;
	}
	if (y1 > y2) {
		int temp = y1;
		y1 = y2;
		y2 = temp;
	}
	//se verifica daca sunt valide coordonatele
	if (x1 < 0 || y1 < 0 || x2 > w || y2 > h || x1 == x2 || y1 == y2) {
		printf("Invalid set of coordinates\n");
		return;
	}
	//se salveaza selectia
	s->x1 = x1;
	s->y1 = y1;
	s->x2 = x2;
	s->y2 = y2;
	printf("Selected %d %d %d %d\n", x1, y1, x2, y2);
}

void select_all(int w, int h, image *s)
{
	//se selecteaza intreaga imagine
	s->x1 = 0;
	s->y1 = 0;
	s->x2 = w;
	s->y2 = h;
}

void crop(int *w, int *h, image ***pixels, image *s)
{
	//se calculeaza noile dimenisiuni in urma decuparii imaginii
	int new_width = s->x2 - s->x1;
	int new_height = s->y2 - s->y1;
	if (new_width < 0 || new_height < 0) {
		printf("Invalid selection dimensions for cropping\n");
		return;
	}
	//se ia o matrice auxiliara pentru noii pixeli
	image **new_pixels = (image **)malloc(new_height * sizeof(image *));
	if (!new_pixels) {
		fprintf(stderr, "malloc failed\n");
		exit(0);
	}
	for (int i = 0; i < new_height; i++) {
		new_pixels[i] = (image *)malloc(new_width * sizeof(image));
		if (!new_pixels[i]) {
			fprintf(stderr, "malloc failed\n");
			for (int j = 0; j < i; j++) {
				free(new_pixels[j]);
			}
			free(new_pixels);
			exit(0);
		}
	}
	//construim noua matrice cu pixelii corespunzatori
	for (int i = 0; i < new_height; i++) {
		for (int j = 0; j < new_width; j++) {
			new_pixels[i][j] = (*pixels)[s->y1 + i][s->x1 + j];
		}
	}
	free_pixels(pixels, *h);
	//se actualizeaza datele
	*pixels = new_pixels;
	*w = new_width;
	*h = new_height;
	s->x1 = 0;
	s->y1 = 0;
	s->x2 = new_width;
	s->y2 = new_height;
	printf("Image cropped\n");
}

void rotate(int angle, int *w, int *h, image ***pixels, image *s)
{
	if (!*pixels) {
		printf("No image loaded\n");
		return;
	}
	if (angle % 90 != 0) {
		printf("Unsupported rotation angle\n");
		return;
	}
	//se calculeaza numarul de rotatii de 90 de grade necesare
	int r = (angle / 90) % 4;
	if (r < 0)
		r = r + 4;
	//se calculeaza dimensiunile selectiei
	int sel_w = s->x2 - s->x1;
	int sel_h = s->y2 - s->y1;
	//se ia in considerare cazul in care selectia este intreaga imagine
	if (s->x1 == 0 && s->y1 == 0 && s->x2 == *w && s->y2 == *h) {
		for (int k = 0; k < r; k++) {
			//se ia o matrice auxiliara pentru noii pixeli
			image **rot_pix = (image **)malloc(*w * sizeof(image *));
			for (int i = 0; i < *w; i++)
				rot_pix[i] = (image *)malloc(*h * sizeof(image));
			//se realizeaza rotatia
			for (int i = 0; i < *h; i++) {
				for (int j = 0; j < *w; j++) {
					rot_pix[j][*h - 1 - i] = (*pixels)[i][j];
				}
			}
			for (int i = 0; i < *h; i++) {
				free((*pixels)[i]);
			}
			free(*pixels);
			//se actualizeaza datele
			*pixels = rot_pix;
			int x = *w;
			*w = *h;
			*h = x;
			x = s->x2;
			s->x2 = s->y2;
			s->y2 = x;
		}
		printf("Rotated %d\n", angle);
	} else if (sel_w == sel_h) {//cazul in care selectia este patrata
		for (int k = 0; k < r; k++) {
			//se ia o matrice auxiliara pentru noii pixeli
			image **rot_pix = (image **)malloc(sel_w * sizeof(image *));
			for (int i = 0; i < sel_w; i++) {
				rot_pix[i] = (image *)malloc(sel_w * sizeof(image));
			}
			//se realizeaza rotatia
			for (int i = 0; i < sel_w; i++) {
				for (int j = 0; j < sel_w; j++) {
					rot_pix[j][sel_w - i - 1] = (*pixels)[s->y1 + i][s->x1 + j];
				}
			}
			//se copiaza matricea auxiliara in imaginea originala
			for (int i = 0; i < sel_h; i++) {
				for (int j = 0; j < sel_w; j++) {
					(*pixels)[s->y1 + i][s->x1 + j] = rot_pix[i][j];
				}
			}
			for (int i = 0; i < sel_w; i++) {
				free(rot_pix[i]);
			}
			free(rot_pix);
		}
		printf("Rotated %d\n", angle);
	} else
		printf("The selection must be square\n");
}

void clamp(int *value)
{
	//se impune ca valoarea sa fie cuprinsa in intervalul [0,255]
	if (*value < 0)
		*value = 0;
	if (*value > 255)
		*value = 255;
}

int build_kernel(char *par, int kernel[3][3])
{
	int kernel_sum = 0;
	//se alege tipul kernel-ului in functie de parametru
	//se calculeaza si suma, tot in functie de parametru
	if (strstr(par, "EDGE")) {
		int temp[3][3] = {{-1, -1, -1}, {-1, 8, -1}, {-1, -1, -1}};
		memcpy(kernel, temp, sizeof(temp));
	} else if (strstr(par, "SHARPEN")) {
		int temp[3][3] = {{0, -1, 0}, {-1, 5, -1}, {0, -1, 0}};
		memcpy(kernel, temp, sizeof(temp));
	} else if (strstr(par, "GAUSSIAN_BLUR")) {
		int temp[3][3] = {{1, 2, 1}, {2, 4, 2}, {1, 2, 1}};
		memcpy(kernel, temp, sizeof(temp));
		kernel_sum = 16;
	} else if (strstr(par, "BLUR")) {
		int temp[3][3] = {{1, 1, 1}, {1, 1, 1}, {1, 1, 1}};
		memcpy(kernel, temp, sizeof(temp));
		kernel_sum = 9;
	}
	return kernel_sum;
}

void apply(char *command, image ***pixels, int w, int h, image *s, char *type)
{
	if (!*pixels) {
		printf("No image loaded\n");
		return;
	}
	if (strcmp(type, "P2") == 0 || strcmp(type, "P5") == 0) {
		printf("Easy, Charlie Chaplin\n");
		return;
	}
	char par[15];
	//se determina parametrul
	if (strstr(command, "EDGE"))
		strcpy(par, "EDGE");
	else if (strstr(command, "SHARPEN"))
		strcpy(par, "SHARPEN");
	else if (strstr(command, "GAUSSIAN_BLUR"))
		strcpy(par, "GAUSSIAN_BLUR");
	else if (strstr(command, "BLUR"))
		strcpy(par, "BLUR");
	else if (pixels)
		strcpy(par, "INVALID");
	if (strcmp(par, "INVALID") == 0) {
		printf("APPLY parameter invalid\n");
		return;
	}
	//se construieste kernel-ul
	int kernel[3][3];
	int sum = build_kernel(par, kernel);
	//se ia o matrice auxiliara pentru noii pixeli
	image **new_pixels = (image **)malloc(h * sizeof(image *));
	for (int i = 0; i < h; i++) {
		new_pixels[i] = (image *)malloc(w * sizeof(image));
		for (int j = 0; j < w; j++) {
			new_pixels[i][j] = (*pixels)[i][j];
		}
	}
	//se aplica kernel-ul pe selectie
	for (int i = s->y1; i < s->y2; i++) {
		for (int j = s->x1; j < s->x2; j++) {
			if (i > 0 && i < h - 1 && j > 0 && j < w - 1) {
				int sumR = 0, sumG = 0, sumB = 0;
				for (int ki = -1; ki <= 1; ki++) {
					for (int kj = -1; kj <= 1; kj++) {
						int ni = i + ki;
						int nj = j + kj;
						sumR += (*pixels)[ni][nj].R * kernel[ki + 1][kj + 1];
						sumG += (*pixels)[ni][nj].G * kernel[ki + 1][kj + 1];
						sumB += (*pixels)[ni][nj].B * kernel[ki + 1][kj + 1];
					}
				}
				if (sum > 0) {
					if (strstr(par, "BLUR")) {
						sumR /= sum;
						sumG /= sum;
						sumB /= sum;
					}
				}
				clamp(&sumR);
				clamp(&sumG);
				clamp(&sumB);
				new_pixels[i][j].R = sumR;
				new_pixels[i][j].G = sumG;
				new_pixels[i][j].B = sumB;
			}
		}
	}
	//se copiaza matricea auxiliara in imaginea originala
	for (int i = s->y1; i < s->y2; i++) {
		for (int j = s->x1; j < s->x2; j++) {
			(*pixels)[i][j] = new_pixels[i][j];
		}
	}
	for (int i = 0; i < h; i++) {
		free(new_pixels[i]);
	}
	free(new_pixels);
	printf("APPLY %s done\n", par);
}

void histogram(int w, int h, int val, image **pixels, char *type, int x, int y)
{
	if (strcmp(type, "P2") != 0 && strcmp(type, "P5") != 0) {
		printf("Black and white image needed\n");
		return;
	}
	//se verifica daca numarul de bins este putere a lui 2 din [2,256]
	if ((y & (y - 1)) != 0 || y < 2 || y > 256) {
		printf("Invalid number of bins\n");
		return;
	}
	//se calculeaza dimensiunea fiecarui bin
	int size = (val + 1) / y;
	//se calculeaza histograma
	int *hist = calloc(y, sizeof(int));
	for (int i = 0; i < h; i++) {
		for (int j = 0; j < w; j++) {
			int bin = pixels[i][j].p / size;
			if (bin >= y)
				bin = y - 1;
			hist[bin]++;
		}
	}
	//se determina frecventa maxima de normalizare
	int max_count = 0;
	for (int i = 0; i < y; i++) {
		if (hist[i] > max_count)
			max_count = hist[i];
	}
	//se afiseaza histograma conform cerintei
	for (int i = 0; i < y; i++) {
		int stars = (hist[i] * x) / max_count;
		printf("%d	|	", stars);
		for (int j = 0; j < stars; j++) {
			printf("*");
		}
		printf("\n");
	}
	free(hist);
}

void equalize(int w, int h, int val, image **pixels, char *type)
{
	if (strcmp(type, "P2") != 0 && strcmp(type, "P5") != 0) {
		printf("Black and white image needed\n");
		return;
	}
	//se calculeaza histograma
	int *histogram = calloc(val + 1, sizeof(int));
	if (!histogram) {
		fprintf(stderr, "Memory allocation failed\n");
		return;
	}
	for (int i = 0; i < h; i++) {
		for (int j = 0; j < w; j++) {
			histogram[pixels[i][j].p]++;
		}
	}
	//se calculeaza histograma cumulativa
	int *c = calloc(val + 1, sizeof(int));
	c[0] = histogram[0];
	for (int i = 1; i <= val; i++) {
		c[i] = c[i - 1] + histogram[i];
	}
	int A = w * h;
	//se actualizeaza valorile pixelilor pe baza histogramelor
	for (int i = 0; i < h; i++) {
		for (int j = 0; j < w; j++) {
			pixels[i][j].p = (c[pixels[i][j].p] * val) / A;
		}
	}
	free(histogram);
	free(c);
	printf("Equalize done\n");
}

void save(char *command, char *type, int w, int h, int val, image **pixels)
{
	//se extrage numele fisierului si se identifica existenta optiunii ascii
	char output_file[NMAX];
	char *t = strtok(command, " ");
	t = strtok(NULL, " ");
	if (t) {
		strcpy(output_file, t);
	}
	t = strtok(NULL, " ");
	int ok = 0;
	if (t && strstr(t, "ascii")) {
		ok = 1;
	}
	if (output_file[strlen(output_file) - 1] == '\n') {
		output_file[strlen(output_file) - 1] = '\0';
	}
	FILE *file = fopen(output_file, "w");
	if (!file) {
		printf("Failed to save %s\n", output_file);
		return;
	}
	//se scriu datele in functie de tipul imaginii si de optiunea ascii
	if (ok == 1) {
		if (strcmp(type, "P5") == 0) {
			fprintf(file, "P2\n");
		} else if (strcmp(type, "P6") == 0) {
			fprintf(file, "P3\n");
		} else {
			fprintf(file, "%s\n", type);
		}
	} else {
		if (strcmp(type, "P2") == 0) {
			fprintf(file, "P5\n");
		} else if (strcmp(type, "P3") == 0) {
			fprintf(file, "P6\n");
		} else {
			fprintf(file, "%s\n", type);
		}
	}
	fprintf(file, "%d %d\n", w, h);
	fprintf(file, "%d\n", val);
	if (strcmp(type, "P3") == 0 || strcmp(type, "P6") == 0) {
		if (ok == 1) {
			for (int i = 0; i < h; i++) {
				for (int j = 0; j < w; j++) {
					fprintf(file, "%d ", pixels[i][j].R);
					fprintf(file, "%d ", pixels[i][j].G);
					fprintf(file, "%d ", pixels[i][j].B);
				}
				fprintf(file, "\n");
			}
		} else {
			for (int i = 0; i < h; i++) {
				for (int j = 0; j < w; j++) {
					fputc(pixels[i][j].R, file);
					fputc(pixels[i][j].G, file);
					fputc(pixels[i][j].B, file);
				}
			}
		}
	} else if (strcmp(type, "P2") == 0 || strcmp(type, "P5") == 0) {
		if (ok == 1) {
			for (int i = 0; i < h; i++) {
				for (int j = 0; j < w; j++) {
					fprintf(file, "%d ", pixels[i][j].p);
				}
				fprintf(file, "\n");
			}
		} else {
			for (int i = 0; i < h; i++) {
				for (int j = 0; j < w; j++) {
					fputc(pixels[i][j].p, file);
				}
			}
		}
	}
	fclose(file);
	printf("Saved %s\n", output_file);
}

int main(void)
{
	char command[NMAX], file_name[NMAX], type[3];
	int width, height, max_val;
	image **pixels = NULL;//pointer pentru matricea pixelilor imaginii
	image curr_sel = {0, 0, 0, 0, -1, -1, -1, -1};//initializarea selectiei
	fgets(command, sizeof(command), stdin);
	while (!strstr(command, "EXIT")) {
		if (strstr(command, "LOAD")) {//incarcarea unei imagini
			if (sscanf(command, "LOAD %s", file_name) == 1) {
				load(file_name, type, &width, &height, &max_val, &pixels);
				select_all(width, height, &curr_sel);
			}
		} else if (strstr(command, "SELECT ALL")) {//selectare totala imagine
			if (pixels) {
				select_all(width, height, &curr_sel);
				printf("Selected ALL\n");
			} else
				printf("No image loaded\n");
		} else if (strstr(command, "SELECT")) {//selectare regiune
			int x1, y1, x2, y2;
			if (sscanf(command, "SELECT %d %d %d %d", &x1, &y1, &x2, &y2) == 4)
				if (pixels)
					selection(x1, y1, x2, y2, width, height, &curr_sel);
				else
					printf("No image loaded\n");
			else
				printf("Invalid command\n");
		} else if (strstr(command, "CROP")) {//decuparea imaginii
			if (pixels)
				crop(&width, &height, &pixels, &curr_sel);
			else
				printf("No image loaded\n");
		} else if (strstr(command, "APPLY")) {//aplicarea unui filtru
			if (pixels) {
				command[strlen(command) - 1] = '\0';
				if (strlen(command) == 5)
					printf("Invalid command\n");
				else
					apply(command, &pixels, width, height, &curr_sel, type);
			} else
				printf("No image loaded\n");
		} else if (strstr(command, "ROTATE")) {//rotirea imaginii
			int angle;
			if (sscanf(command, "ROTATE %d", &angle) == 1) {
				if (pixels)
					rotate(angle, &width, &height, &pixels, &curr_sel);
				else
					printf("No image loaded\n");
			}
		} else if (strstr(command, "HISTOGRAM")) {//generare histograma
			if (pixels) {
				int x, y;
				char extra;
				if (sscanf(command, "HISTOGRAM %d %d %c", &x, &y, &extra) == 2)
					histogram(width, height, max_val, pixels, type, x, y);
				else
					printf("Invalid command\n");
			} else
				printf("No image loaded\n");
		} else if (strstr(command, "EQUALIZE")) {//egalizare imagine alb-negru
			if (pixels)
				equalize(width, height, max_val, pixels, type);
			else
				printf("No image loaded\n");
		} else if (strstr(command, "SAVE")) {//salvare imagine
			if (!pixels)
				printf("No image loaded\n");
			else
				save(command, type, width, height, max_val, pixels);
		} else
			printf("Invalid command\n");
		fgets(command, sizeof(command), stdin);
	}
	if (pixels)
		free_pixels(&pixels, height);
	else
		printf("No image loaded");
	return 0;
}
