#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <errno.h>
#include <cairo.h>
#include <execinfo.h>
#include <signal.h>
#include <unistd.h>
#include "view.h"

void SIGSEGV_handler(int sig) {
   void *array[10];
   size_t size;

   // get void*'s for all entries on the stack
   size = backtrace(array, 10);
            
   // print out all the frames to stderr
   fprintf(stderr, "Error: signal %d:\n", sig);
   backtrace_symbols_fd(array, size, STDERR_FILENO);
   exit(1);
}

int main(int argc, const char *argv[])
{
   signal(SIGSEGV, SIGSEGV_handler);
   // Create list of ball pointers.
   //FILE * inputf = fopen("example_starcode_output.txt", "r");
   //int n_balls;
   //ball_t * ball_list = create_ball_list(inputf, &n_balls);

   /* Begin test code */
   const int n_balls = 8;
   ball_t * ball_list[8]; // in the stack
   ball_list[0] = malloc(BALL_SIZE(3));
   ball_list[1] = malloc(BALL_SIZE(0));
   ball_list[2] = malloc(BALL_SIZE(0));
   ball_list[3] = malloc(BALL_SIZE(0));
   ball_list[4] = malloc(BALL_SIZE(2));
   ball_list[5] = malloc(BALL_SIZE(1));
   ball_list[6] = malloc(BALL_SIZE(0));
   ball_list[7] = malloc(BALL_SIZE(0));

   // Cluster 1
   ball_list[0]->size = 1000;
   ball_list[0]->n_children = 3;
   ball_list[0]->root = ball_list[0];
   ball_list[0]->children[0] = ball_list[1];
   ball_list[0]->children[1] = ball_list[2];
   ball_list[0]->children[2] = ball_list[3];

   ball_list[1]->size = 10;
   ball_list[1]->n_children = 0;
   ball_list[1]->root = ball_list[0];
   
   ball_list[2]->size = 5;
   ball_list[2]->n_children = 0;
   ball_list[2]->root = ball_list[0];

   ball_list[3]->size = 30;
   ball_list[3]->n_children = 0;
   ball_list[3]->root = ball_list[0];

   // Cluster 2
   ball_list[4]->size = 1500;
   ball_list[4]->n_children = 2;
   ball_list[4]->root = ball_list[4];
   ball_list[4]->children[0] = ball_list[5];
   ball_list[4]->children[1] = ball_list[6];

   ball_list[5]->size = 20;
   ball_list[5]->n_children = 1;
   ball_list[5]->root = ball_list[4];
   ball_list[5]->children[0] = ball_list[7];

   ball_list[6]->size = 15;
   ball_list[6]->n_children = 0;
   ball_list[6]->root = ball_list[4];

   ball_list[7]->size = 8;
   ball_list[7]->n_children = 0;
   ball_list[7]->root = ball_list[4];
   /* End test code */

   // Initialize ball positions.
   // Define canvas size and generate random positions in it.
   int canvas_size[2] = { WIDTH * ASPECT, WIDTH };
   srand(time(NULL));
   for (int i = 0; i < n_balls; i++) {
      ball_list[i]->position[0] = rand() * RAND_FACTOR * ASPECT;
      ball_list[i]->position[1] = rand() * RAND_FACTOR;
   }

   // Initialize and run physics loop.
   double temperature = 1.0;
   double new_temperature = 1.0;
   double epsilon = 1.0;//e-12;
   double kt = 1.0;
int temp_iter = 1;
//printf("temperature before iteration %d: %e\n", temp_iter, temperature);
   //while (temperature > epsilon) {
   while (temp_iter < 10000) {
      // Compute the forces among the balls.
      // Reinitialize forces at each iteration.
      for (int i = 0; i < n_balls; i++) {
         ball_list[i]->force[0] = 0.0;
         ball_list[i]->force[1] = 0.0;
      }
      for (int i = 0; i < n_balls; i++) {
         ball_t * ball = ball_list[i];
         // Compute elastic... 
         for (int j = 0; j < ball->n_children; j++) {
            compute_force(ball, ball->children[j], 0);
         }
         // ...and electric forces.
         for (int k = i+1; k < n_balls; k++) {
            // Physically isolate the different clusters.
            if (ball->root == ball_list[k]->root) {
               compute_force(ball, ball_list[k], 1);
            }
         }
      }

      // Move balls and update new temperature.
      new_temperature = 0.0;
      for (int i = 0; i < n_balls; i++) {
         new_temperature += move_ball(ball_list[i], kt);
      }
      
      // Adjust temperature according to new temperature.
      //if (new_temperature > temperature) {
      //   new_temperature = kt * temperature / 2;
      //   kt /= 2.0;
      //}
      //temperature = kt * new_temperature;
      temperature = new_temperature;
//temperature = 1.0/temp_iter;
temp_iter++;
//printf("temperature before iteration %d: %e\n", temp_iter, temperature);
//draw_cairo_env(cr, n_balls, ball_list, 0);
   }

   // Define clusters and bubble-plot them.
   int n_clusters = 0;
   cluster_t ** cluster_list =
      create_cluster_list(n_balls, ball_list, &n_clusters);
   qsort(cluster_list, n_clusters, sizeof(ball_t *), compar);
   // Iterate over the new list and do bubble plot
   // The bubble stuff defines cluster->displacement!
   spiralize_displacements(n_clusters, cluster_list, canvas_size);
   move_clusters(n_balls, n_clusters, ball_list, cluster_list);

   // Draw all balls and bonds.
   int max_size[2];
   measure_space(n_balls, max_size, ball_list);
   //int rel_size[2];
   //for (int i = 0; i < 2; i++) { rel_size[i] = canvas_size[i] / max_size[i]; }
   cairo_surface_t * surface = cairo_image_surface_create(
         CAIRO_FORMAT_ARGB32, canvas_size[0], canvas_size[1]);
   cairo_t * cr = cairo_create(surface);
   cairo_set_source_rgb(cr, 1, 1, 1);
   cairo_paint(cr);

   draw_cairo_env(cr, n_balls, ball_list);
   cairo_surface_write_to_png(surface, "example_starcode_image.png");

   return 0;
}

//ball_t *
//create_ball_list
//(
//   //FILE * inputf,
//   int  * n_balls
//)
//{
//   *n_balls = 0;
//   size_t list_size = 1024;
//   ball_t * ball_list = malloc(list_size * sizeof(ball_t *));
//   while (/*read line and store values*/-1 != -1) {
//      ball_t * ball = new_ball(/*ball.n_children*/);
//      n_balls++;
//      if (n_balls >= current_size) {
//         list_size *= 2;
//         ball_list = realloc(ball_list, list_size);
//      }
//   }
//
//   return ball_list;
//
//}

//ball_t *
//new_ball
//(
//   int n_children
//)
//{
//   ball_t * ball = malloc(BALL_SIZE(n_children));
//   //if (ball == NULL) {
//   //   fprintf(stderr, "Error in ball malloc: %s\n", strerror(errno));
//   //}
//   // TODO: fill ball with info.
//
//   return ball;
//}

double
norm
(
   double x_coord,
   double y_coord
)
{
   return sqrt(pow(x_coord, 2) + pow(y_coord, 2));
}

double
electric
(
   ball_t * ball1,
   ball_t * ball2,
   double   dist
)
{
   double ke = 5.0e1;
   // Coulomb's law.
   return ke * ball1->size * ball2->size / pow(dist, 2);
}

double
elastic
(
   double dist
)
{
   double k = 10.0;
   // Hooke's law.
   return -k * dist;
}

void
compute_force
(
   ball_t * ball1,
   ball_t * ball2,
   int      force_type
)
{
   // Relative ball positions determine the sign (+/-) of the unit vector.
   double x_dist = ball2->position[0] - ball1->position[0];
   double y_dist = ball2->position[1] - ball1->position[1];
   double v_norm = norm(x_dist, y_dist);
   double force;
   if (force_type == 0) {
      force = elastic(v_norm);
   } else if (force_type == 1) {
      force = electric(ball1, ball2, v_norm);
   }
   double u_vect[2] = { x_dist / v_norm, y_dist / v_norm };
   double x_force = force * u_vect[0];
   double y_force = force * u_vect[1];
   ball1->force[0] += -x_force;
   ball2->force[0] +=  x_force;
   ball1->force[1] += -y_force;
   ball2->force[1] +=  y_force;
}

double
move_ball
(
   ball_t * ball,
   double   kt
)
{
   double x_movement = ball->force[0] / ball->size * pow(kt, 2);
   double y_movement = ball->force[1] / ball->size * pow(kt, 2);
   if (x_movement > 1) { x_movement = 1; }
   else if (x_movement < -1) {x_movement = -1;}
   if (y_movement > 1) { y_movement = 1; }
   else if (y_movement < -1) {y_movement = -1;}
   ball->position[0] += x_movement;
   ball->position[1] += y_movement;

   return norm(x_movement, y_movement);
}

int
compar
(
   const void * elem1,
   const void * elem2
)
{
   ball_t * ball1 = (ball_t *) elem1;
   ball_t * ball2 = (ball_t *) elem2;
   return (ball1->size > ball2->size) ? -1 : 1; // Descending order.
}

cluster_t **
create_cluster_list
(
   int       n_balls,
   ball_t ** ball_list,
   int     * n_clusters
)
{
   *n_clusters = 0;
   int list_size  = 1000;
   cluster_t ** cluster_list = malloc(list_size * sizeof(cluster_t *));
   // Define clusters root identity and position.
   for (int i = 0; i < n_balls; i++) {
      if (ball_list[i] == ball_list[i]->root) {
         cluster_list[*n_clusters] = malloc(sizeof(cluster_t));
         cluster_list[*n_clusters]->root = ball_list[i]->root;
         cluster_list[*n_clusters]->position[0] = ball_list[i]->position[0];
         cluster_list[*n_clusters]->position[1] = ball_list[i]->position[1];
         (*n_clusters)++;
         if (*n_clusters >= list_size) {
            list_size *= 2;
            cluster_list =
               realloc(cluster_list, list_size * sizeof(cluster_t *));
         }
      }
   }
   // Define clusters (center) position and radius.
   for (int i = 0; i < *n_clusters; i++) {
      int cluster_size = 0;
      // This initial position is just the position of the root ball.
      //double x_pos = cluster_list[i];//->position[0];
      //double y_pos = cluster_list[i];//->position[1];
      //double  min_pos[2] = { x_pos, y_pos };
      //double  max_pos[2] = { x_pos, y_pos };
      double mean_pos[2] = { 0.0, 0.0 };
      for (int j = 0; j < n_balls; j++) {
         if (cluster_list[i]->root == ball_list[j]->root) {
            mean_pos[0] += ball_list[j]->position[0];
            mean_pos[1] += ball_list[j]->position[1];
            cluster_size++;
         } 
      }
      // Now position is the central position of the cluster.
      cluster_list[i]->position[0] = mean_pos[0] / cluster_size;
      cluster_list[i]->position[1] = mean_pos[1] / cluster_size;
      // Compute the radius.
      cluster_list[i]->radius = 0.0;
      double radius = 0.0;
      for (int j = 0; j < n_balls; j++) {
         ball_t * ball = ball_list[j];
         if (cluster_list[i]->root == ball->root) {
            double x_dist = cluster_list[i]->position[0] - ball->position[0];
            double y_dist = cluster_list[i]->position[1] - ball->position[1];
            double radius = norm(x_dist, y_dist) + sqrt(ball->size / PI);
            if (radius > cluster_list[i]->radius) {
               cluster_list[i]->radius = radius;
            }
         }
      }
   }

   return cluster_list;
}

void
spiralize_displacements
(
   int n_clusters, 
   cluster_t ** cluster_list,
   int * canvas_size
)
{
   double center[2] = { canvas_size[0] / 2.0, canvas_size[1] / 2.0 };
   double step = 0.01; // Step along the spiral and padding between clusters.
   // Place the first cluster in the center of the canvas.
   cluster_list[0]->displacement[0] = center[0] - cluster_list[0]->position[0];
   cluster_list[0]->displacement[1] = center[1] - cluster_list[0]->position[1];
   cluster_list[0]->position[0] = center[0];
   cluster_list[0]->position[1] = center[1];
   for (int i = 1; i < n_clusters; i++) {
      cluster_t * clust1 = cluster_list[i];
      double x_pos;
      double y_pos;
      double distance = 0.0; // Distance from center, along a spiral line.
      double phase = 2 * PI * rand() / RAND_MAX;
      int overlap = 1;
      while (overlap) {
         overlap = 0;
         x_pos = center[0] + distance * cos(distance + phase); 
         y_pos = center[1] + distance * sin(distance + phase); 
         for (int j = 0; j < i; j++) {
            cluster_t * clust2 = cluster_list[j];
            double x_dist = x_pos - clust2->position[0];
            double y_dist = y_pos - clust2->position[1];
            double radii = clust1->radius + clust2->radius;
            if (norm(x_dist, y_dist) - radii < step) {
               overlap = 1;
               distance += step;
               break;
            }
         }
      }
      clust1->displacement[0] = x_pos - clust1->position[0];
      clust1->displacement[1] = y_pos - clust1->position[1];
      clust1->position[0] = x_pos;
      clust1->position[1] = y_pos;
   }
}

void
move_clusters
(
   int          n_balls,
   int          n_clusters,
   ball_t    ** ball_list,
   cluster_t ** cluster_list
)
{
   for (int i = 0; i < n_balls; i++) {
      for (int j = 0; j < n_clusters; j++) {
         if (ball_list[i]->root == cluster_list[j]->root) {
            ball_list[i]->position[0] += cluster_list[j]->displacement[0];
            ball_list[i]->position[1] += cluster_list[j]->displacement[1];
         }
      }
   }
}

void
measure_space
(
   int       n_balls,
   int     * max_size,
   ball_t ** ball_list
)
{
   int x_max = 0;
   int y_max = 0;
   for (int i = 0; i < n_balls; i++) {
      int x_pos = ball_list[i]->position[0];
      int y_pos = ball_list[i]->position[1];
      if (x_pos > x_max) { x_max = x_pos; }
      if (y_pos > y_max) { y_max = y_pos; }
   }
   max_size[0] = x_max;
   max_size[1] = y_max;
}

void
draw_edges
(
   cairo_t * cr,
   ball_t  * ball
)
{
   double x_pos = ball->position[0];// * rel_size[0];
   double y_pos = ball->position[1];// * rel_size[1];
   for (int j = 0; j < ball->n_children; j++) {
      double child_x_pos = ball->children[j]->position[0];// * rel_size[0];
      double child_y_pos = ball->children[j]->position[1];// * rel_size[1];
      cairo_set_line_width(cr, 1);
      cairo_set_source_rgb(cr, 0, 0, 0);
      cairo_move_to(cr, x_pos, y_pos);
      cairo_line_to(cr, child_x_pos, child_y_pos);
      cairo_stroke(cr);
   }
}

void
draw_circles
(
   cairo_t * cr,
   ball_t  * ball
)
{
   double x_pos = ball->position[0];// * rel_size[0];
   double y_pos = ball->position[1];// * rel_size[1];
   double radius = sqrt(ball->size / PI);
   // Draw the circle.
   cairo_set_source_rgb(cr, 0.85, 0.85, 0.85);
   cairo_arc(cr, x_pos, y_pos, radius, 0, 2*PI);
   cairo_close_path(cr);
   cairo_fill(cr);
   // And the outline.
   cairo_set_source_rgb(cr, 0, 0, 0);
   cairo_arc(cr, x_pos, y_pos, radius, 0, 2*PI);
   cairo_close_path(cr);
   cairo_stroke(cr);
}

void
draw_cairo_env
(
   cairo_t * cr,
   int       n_balls,
   ball_t ** ball_list
)
{
   // Paint the background.
   //cairo_set_source_rgb(cr, 1, 1, 1);
   //cairo_paint(cr);
   for (int i = 0; i < n_balls; i++) {
      ball_t * ball = ball_list[i];
      // And then the graphs.
      draw_edges(cr, ball);
      draw_circles(cr, ball);
   }
}