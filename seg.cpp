#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <opencv2/opencv.hpp>
#include <algorithm>
#include <vector>

using namespace std;
using namespace cv;

typedef struct { unsigned char r, g, b; } rgb;

// disjoint-set forests using union-by-rank and path compression (sort of).
typedef struct {
	int rank;
	int p;
	int size;
} uni_elt;

class universe {
public:
	universe(int elements);
	~universe();
	int find(int x);  
	void join(int x, int y);
	int size(int x) const { return elts[x].size; }
	int num;
private:
	uni_elt *elts;
};

universe::universe(int elements) {
	elts = new uni_elt[elements];
	num = elements;
	for (int i = 0; i < elements; i++) {
		elts[i].rank = 0;
		elts[i].size = 1;
		elts[i].p = i;
	}
}
  
universe::~universe() {
	delete [] elts;
}

int universe::find(int x) {
	int y = x;
	while (y != elts[y].p)
		y = elts[y].p;
	elts[x].p = y;
	return y;
}

void universe::join(int x, int y) {
	if (elts[x].rank > elts[y].rank) {
		elts[y].p = x;
		elts[x].size += elts[y].size;
	} else {
		elts[x].p = y;
		elts[y].size += elts[x].size;
		if (elts[x].rank == elts[y].rank)
			elts[y].rank++;
	}
	num--;
}

typedef struct {
	float w;
	int a, b;
} edge;

bool operator<(const edge &a, const edge &b) {
	return a.w < b.w;
}

/* Segment a graph: Returns a disjoint-set forest representing the segmentation.
 *
 * num_vertices: number of vertices in graph.
 * num_edges: number of edges in graph
 * edges: array of edges.
 * k: constant for treshold function.
 */
universe *segment_graph(int num_vertices, int num_edges, edge *edges, float k) { 
	std::sort(edges, edges + num_edges);		// sort edges by weight
	universe *u = new universe(num_vertices);	// make a disjoint-set forest

	// init thresholds
	float *threshold = new float[num_vertices];
	for (int i = 0; i < num_vertices; i++)
		threshold[i] = k;

	// for each edge, in non-decreasing weight order...
	for (int i = 0; i < num_edges; i++) {
		edge *pedge = &edges[i];

		// components conected by this edge
		int a = u->find(pedge->a);
		int b = u->find(pedge->b);
		if ((a != b) && (pedge->w <= threshold[a]) && (pedge->w <= threshold[b])) {
			u->join(a, b);
			a = u->find(a);
			threshold[a] = pedge->w + k/(u->size(a));
		}
	}
	delete threshold;
	return u;
}

// dissimilarity measure between pixels
static inline float diff(Mat img, int x1, int y1, int x2, int y2) {
	return abs(img.at<Vec3b>(y1,x1)[0] - img.at<Vec3b>(y2,x2)[0]) + abs(img.at<Vec3b>(y1,x1)[1] - img.at<Vec3b>(y2,x2)[1]) + abs(img.at<Vec3b>(y1,x1)[2] - img.at<Vec3b>(y2,x2)[2]);
}

/* sigma: to smooth the image.
 * k: constant for treshold function.
 * min_size: minimum component size (enforced by post-processing stage).
 * num_ccs: number of connected components in the segmentation. */
void segment_image(Mat orig_img, Mat seg_img, float sigma, float k, int min_size, int *num_ccs) {
	int width = orig_img.cols;
	int height = orig_img.rows;

	GaussianBlur(orig_img, seg_img, Size(sigma,sigma), 0, 0);

	// build graph
	edge *edges = new edge[width*height*4];
	int num = 0;
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			if (x < width-1) {
				edges[num].a = y * width + x;
				edges[num].b = y * width + (x+1);
				edges[num].w = diff(seg_img, x, y, x+1, y);
				num++;
			}
			if (y < height-1) {
				edges[num].a = y * width + x;
				edges[num].b = (y+1) * width + x;
				edges[num].w = diff(seg_img, x, y, x, y+1);
				num++;
			}
			if ((x < width-1) && (y < height-1)) {
				edges[num].a = y * width + x;
				edges[num].b = (y+1) * width + (x+1);
				edges[num].w = diff(seg_img, x, y, x+1, y+1);
				num++;
			}
			if ((x < width-1) && (y > 0)) {
				edges[num].a = y * width + x;
				edges[num].b = (y-1) * width + (x+1);
				edges[num].w = diff(seg_img, x, y, x+1, y-1);
				num++;
			}
		}
	}

	// segment
	universe *u = segment_graph(width*height, num, edges, k);

	// post process small components
	for (int i = 0; i < num; i++) {
		int a = u->find(edges[i].a);
		int b = u->find(edges[i].b);
		if ((a != b) && ((u->size(a) < min_size) || (u->size(b) < min_size)))
			u->join(a, b);
	}
	delete [] edges;
	*num_ccs = u->num;

	// pick random colors for each component
	rgb *colors = new rgb[width*height];
	for (int i = 0; i < width*height; i++) {
		colors[i].r = (unsigned char)random();
		colors[i].g = (unsigned char)random();
		colors[i].b = (unsigned char)random();
	}

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			int comp = u->find(y * width + x);
			seg_img.at<Vec3b>(y, x)[0] = colors[comp].b;
			seg_img.at<Vec3b>(y, x)[1] = colors[comp].g;
			seg_img.at<Vec3b>(y, x)[2] = colors[comp].r;
		}
	}

	delete [] colors;
	delete u;
}

int main(int argc, char **argv) {
	int num_ccs = 0; 
	float sigma = 41;//0.5;
	float k = 500;
	int min_size = 1024;
	string img_file = "/tmp/Red.png";

	if (argc > 1)
		img_file = argv[1];
	Mat orig_img = imread(img_file);
	Mat seg_img(orig_img.rows, orig_img.cols, CV_8UC3);

	segment_image(orig_img, seg_img, sigma, k, min_size, &num_ccs); 

	imwrite("segment.jpg", seg_img);
	printf("got %d components\n", num_ccs);

	return 0;
}
