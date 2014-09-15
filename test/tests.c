#include "trie.h"
#include "_trie.h"
#include "starcode.h"
#include "_starcode.h"
#include "unittest.h"

// Convenience functions.
int visit(node_t*, char*, int, int, int);
char *to_string (trie_t *trie, char* s) {
   visit(trie->root, s, 0, get_height(trie), 0);
   return s;
}
void reset_gstack(gstack_t **g) {
   for (int i = 0 ; g[i] != TOWER_TOP ; i++) g[i]->nitems = 0;
}

int
visit
(
   node_t *node,
   char *buff,
   int j,
   int maxdepth,
   int depth
)
{
   for (int i = 0 ; i < 6 ; i++) {
      if (node->child[i] != NULL) {
         buff[j++] = untranslate[i];
         if (depth < maxdepth-1) {
            node_t *child = (node_t *) node->child[i];
            j = visit(child, buff, j, maxdepth, depth+1);
         }
         else buff[j++] = '*';
      }
   }
   buff[j++] = '*';
   buff[j] = '\0';
   return j;
}


trie_t *
setup
(void)
//   Construct a very simple trie for testing purposes.                  
{

   const char *string[22] = {
   "AAAAAAAAAAAAAAAAAAAA",
   "AAAAAAAAAANAAAAAAAAA",
   "NAAAAAAAAAAAAAAAAAAN",
   "NNAAAAAAAAAAAAAAAANN",
   "AAAAAAAAAAAAAAAAAAAT",
   "TAAAAAAAAAAAAAAAAAAA",
   "GGATTAGATCACCGCTTTCG",
   "TTGGTATATGTCATAGAAAT",
   "TTCGGAGCGACTAATATAGG",
   "CGAGGCGTATAGTGTTTCCA",
   "ATGCTAGGGTACTCGATAAC",
   "CATACAGTATTCGACTAAGG",
   "TGGAGATGATGAAGAAGACC",
   "GGGAGACTTTTCCAGGGTAT",
   "TCATTGTGATAGCTCGTAAC",
   " GGATATCAAGGGTTACTAG",
   " AAAAAAAAAAAAAAAAAAA",
   "            AAAAAAAA",
   "            NNNNNNNN",
   "                   A",
   "                   N",
   "                    ",
   };

   trie_t * trie = new_trie(20);
   if (trie == NULL) {
      fprintf(stderr, "unittest error: %s:%d\n", __FILE__, __LINE__);
      abort();
   }

   for (int i = 0 ; i < 22 ; i++) {
      void **data = insert_string(trie, string[i]);
      // Set node data to self (non 'NULL').
      if (data != NULL) *data = data;
      else {
         fprintf(stderr, "unittest error: %s:%d\n", __FILE__, __LINE__);
         abort();
      }
   }

   // String representation of the trie as a depth first search
   // traversal. The stars indicate the a series of with no further
   // children. I also use indentation to ease the reading.
   char trie_string_representation[] =
   "NNAAAAAAAAAAAAAAAANN"
    "*******************"
    "AAAAAAAAAAAAAAAAAAN"
   "********************"
   "AAAAAAAAAANAAAAAAAAA"
             "**********"
             "AAAAAAAAAA"
                      "*"
                      "T"
    "*******************"
    "TGCTAGGGTACTCGATAAC"
   "********************"
   "CATACAGTATTCGACTAAGG"
    "*******************"
    "GAGGCGTATAGTGTTTCCA"
   "********************"
   "GGATTAGATCACCGCTTTCG"
     "******************"
     "GAGACTTTTCCAGGGTAT"
   "********************"
   "TAAAAAAAAAAAAAAAAAAA"
    "*******************"
    "CATTGTGATAGCTCGTAAC"
    "*******************"
    "GGAGATGATGAAGAAGACC"
    "*******************"
    "TCGGAGCGACTAATATAGG"
     "******************"
     "GGTATATGTCATAGAAAT"
   "********************"
   " AAAAAAAAAAAAAAAAAAA"
    "*******************"
    "GGATATCAAGGGTTACTAG"
    "*******************"
    "           NNNNNNNN"
               "********"
               "AAAAAAAA"
               "********"
               "       N"
                      "*"
                      "A"
                      "*"
                      " "
   "********************"
   "*";

   // Make sure the trie has the correct structure.
   char buff[1024];
   to_string(trie, buff);
   if (strcmp(trie_string_representation, buff) != 0) {
      fprintf(stderr, "unittest error: %s:%d\n", __FILE__, __LINE__);
      abort();
   }
   if (count_nodes(trie) != 338) {
      fprintf(stderr, "unittest error: %s:%d\n", __FILE__, __LINE__);
      abort();
   }

   return trie;

}

void
teardown
(
   trie_t * trie
)
{
   destroy_trie(trie, DESTROY_NODES_YES, NULL);
}



void
test_base_1
(void)
// Test node creation and destruction.
{

   const char cache[17] = {8,7,6,5,4,3,2,1,0,1,2,3,4,5,6,7,8};
   node_t *node = new_trienode();
   // Check that creation succeeded.
   test_assert_critical(node != NULL);
   // Check initialization.
   test_assert(node->path == 0);
   for (int i = 0 ; i < 6 ; i++) {
      test_assert(node->child[i] == NULL);
   }
   for (int i = 0 ; i < 2*TAU+1 ; i++) {
      test_assert(node->cache[i] == cache[i]);
   }
   
   free(node);

}

void
test_base_2
(void)
// Test 'insert()'.
{
   node_t *root = new_trienode();
   test_assert_critical(root != NULL);

   for (int i = 0 ; i < 6 ; i++) {
      node_t *node = insert(root, i);
      test_assert_critical(node != NULL);
      for (int j = 0 ; j < 6 ; j++) {
         test_assert(node->child[j] == NULL);
      }
      const char cache[17] = {8,7,6,5,4,3,2,1,0,1,2,3,4,5,6,7,8};
      for (int i = 0 ; i < 2*TAU+1 ; i++) {
         test_assert(node->cache[i] == cache[i]);
      }
      test_assert(node->path == i);
      test_assert(root->child[i] == node);
   }

   // Destroy manually.
   for (int i = 0 ; i < 6 ; i++) free(root->child[i]);
   free(root);

}


void
test_base_3
(void)
// Test 'insert_wo_malloc()'.
{
   node_t *root = new_trienode();
   test_assert_critical(root != NULL);

   node_t *nodes = malloc(6 * sizeof(node_t));
   if (nodes == NULL) {
      fprintf(stderr, "unittest error: %s:%d\n", __FILE__, __LINE__);
      exit(EXIT_FAILURE);
   }

   for (int i = 0 ; i < 6 ; i++) {
      node_t *node = insert_wo_malloc(root, i, nodes+i);
      test_assert_critical(node != NULL);
      for (int j = 0 ; j < 6 ; j++) {
         test_assert(node->child[j] == NULL);
      }
      const char cache[17] = {8,7,6,5,4,3,2,1,0,1,2,3,4,5,6,7,8};
      for (int i = 0 ; i < 2*TAU+1 ; i++) {
         test_assert(node->cache[i] == cache[i]);
      }
      test_assert(node->path == i);
      test_assert(root->child[i] == node);
   }

   free(root);
   free(nodes);

}


void
test_base_4
(void)
// Test trie creation and destruction.
{

   for (int height = 1 ; height < M ; height++) {
      trie_t *trie = new_trie(height);
      test_assert_critical(trie != NULL);
      test_assert_critical(trie->root != NULL);
      test_assert_critical(trie->info != NULL);

      test_assert(get_height(trie) == height);

      // Make sure that 'info' is initialized properly.
      info_t *info = trie->info;
      test_assert(((node_t*) *info->pebbles[0]->items) == trie->root);
      for (int i = 1 ; i < M ; i++) {
         test_assert_critical(info->pebbles[i]->items != NULL);
      }

      // Insert 20 random sequences.
      for (int i = 0 ; i < 20 ; i++) {
         char seq[M] = {0};
         for (int j = 0 ; j < height ; j++) {
            seq[j] = untranslate[(int)(5 * drand48())];
         }
         void **data = insert_string(trie, seq);
         test_assert(data != NULL);
         *data = data;
      }
      destroy_trie(trie, DESTROY_NODES_YES, NULL);
      trie = NULL;
   }

   for (int height = 1 ; height < M ; height++) {
      trie_t *trie = new_trie(height);
      test_assert_critical(trie != NULL);
      test_assert_critical(trie->root != NULL);
      test_assert_critical(trie->info != NULL);

      test_assert(get_height(trie) == height);

      // Make sure that 'info' is initialized properly.
      info_t *info = trie->info;
      test_assert(((node_t*) *info->pebbles[0]->items) == trie->root);
      for (int i = 1 ; i < M ; i++) {
         test_assert_critical(info->pebbles[i]->items != NULL);
      }

      // Insert 20 random sequences without malloc.
      node_t *nodes = malloc(20*height * sizeof(node_t));
      if (nodes == NULL) {
         fprintf(stderr, "unittest error: %s:%d", __FILE__, __LINE__);
         exit(EXIT_FAILURE);
      }

      node_t *pos = nodes;
      for (int i = 0 ; i < 20 ; i++) {
         char seq[M] = {0};
         for (int j = 0 ; j < height ; j++) {
            seq[j] = untranslate[(int)(5 * drand48())];
         }
         void **data = insert_string_wo_malloc(trie, seq, &pos);
         test_assert(data != NULL);
         *data = data;
      }
      destroy_trie(trie, DESTROY_NODES_NO, NULL);
      free(nodes);
      trie = NULL;
   }
   
}


void
test_base_5
(void)
// Test 'insert_string()'.
{

   trie_t *trie = new_trie(20);

   void **data = insert_string(trie, "AAAAAAAAAAAAAAAAAAAA");
   test_assert(data != NULL);
   *data = data;
   test_assert(21 == count_nodes(trie));

   destroy_trie(trie, DESTROY_NODES_YES, NULL);

}


void
test_base_6
(void)
// Test 'insert_string_wo_malloc()'.
{
   trie_t *trie = new_trie(20);
   test_assert_critical(trie != NULL);

   node_t *nodes = malloc(19 * sizeof(node_t));
   if (nodes == NULL) {
      fprintf(stderr, "unittest error (%s:%d)\n", __FILE__, __LINE__);
      exit(EXIT_FAILURE);
   }
   node_t *pos = nodes;
   void **data = 
      insert_string_wo_malloc(trie, "AAAAAAAAAAAAAAAAAAAA", &pos);
   test_assert(data != NULL);
   *data = data;
   test_assert(21 == count_nodes(trie));
   // Check that the pointer has been incremented by 19 positions.
   test_assert(19 == (pos - nodes));

   // Cache at initialization.
   const char cache[17] = {8,7,6,5,4,3,2,1,0,1,2,3,4,5,6,7,8};
   // Successive 'paths' members of a line of A in the trie.
   const int paths[20] = {0,1,17,273,4369,69905,1118481,17895697,
      286331153,286331153,286331153,286331153,286331153,286331153,
      286331153,286331153,286331153,286331153,286331153,286331153};

   // Check the integrity of the nodes.
   node_t *node = trie->root;
   for (int i = 0 ; i < 20 ; i++) {
      test_assert_critical(node != NULL);
      test_assert(node->cache != NULL);
      test_assert(node->path == paths[i]);
      test_assert(node->child[0] == NULL);
      test_assert(node->child[1] != NULL);
      for (int j = 2 ; j < 6 ; j++) {
         test_assert(node->child[j] == NULL);
      }
      for (int j = 0 ; j < 2*TAU+1 ; j++) {
         test_assert(node->cache[j] == cache[j]);
      }
      node = node->child[1];
   }

   destroy_trie(trie, DESTROY_NODES_NO, NULL);
   free(nodes);

}


void
test_base_7
(void)
// Test 'new_gstack()' and 'push()'.
{
   gstack_t *gstack = new_gstack();
   test_assert_critical(gstack != NULL);
   test_assert(gstack->items != NULL);
   test_assert(gstack->nslots == GSTACK_INIT_SIZE);
   test_assert(gstack->nitems == 0);

   node_t *node = new_trienode();
   push(node, &gstack);
   test_assert(node == (node_t *) gstack->items[0]);
   test_assert(gstack->nslots == GSTACK_INIT_SIZE);
   test_assert(gstack->nitems == 1);

   free(node);
   free(gstack);
   
}


void
test_base_8
(void)
// Test 'new_tower()'.
{

   for (int i = 1 ; i < 100 ; i++) {
      gstack_t **tower = new_tower(i);
      test_assert_critical(tower != NULL);
      for (int j = 0 ; j < i ; j++) {
         test_assert(tower[j]->nslots == GSTACK_INIT_SIZE);
         test_assert(tower[j]->nitems == 0);
      }
      test_assert(tower[i] == TOWER_TOP);
      destroy_tower(tower);
      tower = NULL;
   }
   
}


void
test_errmsg
(void)
{

   int err;

   // Check error messages in 'insert_string()'.
   redirect_stderr();
   trie_t * trie = new_trie(0);
   unredirect_stderr();
   test_assert(trie == NULL);
   test_assert_stderr("error: the minimum trie height is 1\n");

   trie = setup();

   // Check error messages in 'insert_string()'.
   char too_long_string[M+2];
   for (int i = 0 ; i < M+1 ; i++) too_long_string[i] = 'A';
   too_long_string[M+1] = '\0';
   redirect_stderr();
   void **not_inserted = insert_string(trie, too_long_string);
   unredirect_stderr();

   char string[1024];
   sprintf(string, "error: can only insert string of length %d\n",
         get_height(trie));
   test_assert(not_inserted == NULL);
   test_assert_stderr(string);

   // Check error messages in 'insert_string_wo_malloc()'.
   node_t * dummy;
   redirect_stderr();
   not_inserted = insert_string_wo_malloc(trie, too_long_string, &dummy);
   unredirect_stderr();
   test_assert(not_inserted == NULL);
   test_assert_stderr(string);

   // Check error messages in 'search()'.
   gstack_t **hits = new_tower(4);
   test_assert_critical(hits != NULL);

   redirect_stderr();
   err = search(trie, "AAAAAAAAAAAAAAAAAAAAA", 3, hits, 0, 0);
   unredirect_stderr();
   test_assert(err > 0);
   test_assert_stderr("error: query longer than allowed max\n");
   test_assert(hits[0]->nitems == 0);
   test_assert(hits[1]->nitems == 0);
   test_assert(hits[2]->nitems == 0);
   test_assert(hits[3]->nitems == 0);

   reset_gstack(hits);
   redirect_stderr();
   err = search(trie, " TGCTAGGGTACTCGATAAC", 9, hits, 0, 0);
   unredirect_stderr();
   test_assert(err > 0);
   test_assert_stderr("error: requested tau greater than 8\n");
   test_assert(hits[0]->nitems == 0);
   test_assert(hits[1]->nitems == 0);
   test_assert(hits[2]->nitems == 0);
   test_assert(hits[3]->nitems == 0);

   // Force 'malloc()' to fail and check error messages of constructors.
   redirect_stderr();
   set_alloc_failure_rate_to(1);
   trie_t *trie_fail = new_trie(20);
   reset_alloc();
   unredirect_stderr();
   test_assert(trie_fail == NULL);
   test_assert(check_trie_error_and_reset() >  0);
   test_assert_stderr("error: could not create trie\n");

   redirect_stderr();
   set_alloc_failure_rate_to(1);
   node_t *trienode_fail = new_trienode();
   reset_alloc();
   unredirect_stderr();
   test_assert(trienode_fail == NULL);
   test_assert(check_trie_error_and_reset() >  0);
   test_assert_stderr("error: could not create trie node\n");

   redirect_stderr();
   set_alloc_failure_rate_to(1);
   gstack_t *gstack_fail = new_gstack();
   reset_alloc();
   unredirect_stderr();
   test_assert(gstack_fail == NULL);
   test_assert(check_trie_error_and_reset() >  0);
   test_assert_stderr("error: could not create gstack\n");

   redirect_stderr();
   set_alloc_failure_rate_to(1);
   gstack_t **tower_fail = new_tower(1);
   reset_alloc();
   unredirect_stderr();
   test_assert(tower_fail == NULL);
   test_assert(check_trie_error_and_reset() >  0);
   test_assert_stderr("error: could not create tower\n");

   destroy_tower(hits);
   teardown(trie);

}


void
test_search
(void)
{

   int err;

   trie_t * trie = setup();
   gstack_t **hits = new_tower(4);
   test_assert_critical(hits != NULL);

   err = search(trie, "AAAAAAAAAAAAAAAAAAAA", 3, hits, 0, 18);
   test_assert(err == 0);
   test_assert(check_trie_error_and_reset() == 0);
   test_assert(hits[0]->nitems == 1);
   test_assert(hits[1]->nitems == 4);
   test_assert(hits[2]->nitems == 1);
   test_assert(hits[3]->nitems == 0);

   reset_gstack(hits);
   search(trie, "AAAAAAAAAAAAAAAAAATA", 3, hits, 18, 3);
   test_assert(err == 0);
   test_assert(check_trie_error_and_reset() == 0);
   test_assert(hits[0]->nitems == 0);
   test_assert(hits[1]->nitems == 2);
   test_assert(hits[2]->nitems == 3);
   test_assert(hits[3]->nitems == 1);

   reset_gstack(hits);
   search(trie, "AAAGAAAAAAAAAAAAAATA", 3, hits, 3, 15);
   test_assert(err == 0);
   test_assert(check_trie_error_and_reset() == 0);
   test_assert(hits[0]->nitems == 0);
   test_assert(hits[1]->nitems == 0);
   test_assert(hits[2]->nitems == 2);
   test_assert(hits[3]->nitems == 3);

   reset_gstack(hits);
   search(trie, "AAAGAAAAAAAAAAAGACTG", 3, hits, 15, 15);
   test_assert(err == 0);
   test_assert(check_trie_error_and_reset() == 0);
   test_assert(hits[0]->nitems == 0);
   test_assert(hits[1]->nitems == 0);
   test_assert(hits[2]->nitems == 0);
   test_assert(hits[3]->nitems == 0);

   reset_gstack(hits);
   search(trie, "AAAGAAAAAAAAAAAAAAAA", 3, hits, 15, 0);
   test_assert(err == 0);
   test_assert(check_trie_error_and_reset() == 0);
   test_assert(hits[0]->nitems == 0);
   test_assert(hits[1]->nitems == 2);
   test_assert(hits[2]->nitems == 3);
   test_assert(hits[3]->nitems == 1);

   reset_gstack(hits);
   search(trie, "TAAAAAAAAAAAAAAAAAAA", 3, hits, 0, 19);
   test_assert(err == 0);
   test_assert(check_trie_error_and_reset() == 0);
   test_assert(hits[0]->nitems == 1);
   test_assert(hits[1]->nitems == 2);
   test_assert(hits[2]->nitems == 3);
   test_assert(hits[3]->nitems == 0);

   reset_gstack(hits);
   search(trie, "TAAAAAAAAAAAAAAAAAAG", 3, hits, 19, 0);
   test_assert(err == 0);
   test_assert(check_trie_error_and_reset() == 0);
   test_assert(hits[0]->nitems == 0);
   test_assert(hits[1]->nitems == 1);
   test_assert(hits[2]->nitems == 4);
   test_assert(hits[3]->nitems == 1);

   reset_gstack(hits);
   search(trie, " AAAAAAAAAAAAAAAAAAA", 3, hits, 0, 1);
   test_assert(err == 0);
   test_assert(check_trie_error_and_reset() == 0);
   test_assert(hits[0]->nitems == 1);
   test_assert(hits[1]->nitems == 4);
   test_assert(hits[2]->nitems == 1);
   test_assert(hits[3]->nitems == 0);

   reset_gstack(hits);
   search(trie, "   ATGCTAGGGTACTCGAT", 3, hits, 0, 1);
   test_assert(err == 0);
   test_assert(check_trie_error_and_reset() == 0);
   test_assert(hits[0]->nitems == 0);
   test_assert(hits[1]->nitems == 0);
   test_assert(hits[2]->nitems == 0);
   test_assert(hits[3]->nitems == 1);

   reset_gstack(hits);
   search(trie, " AAAAAAAAAAAAAAAAAAT", 3, hits, 1, 0);
   test_assert(err == 0);
   test_assert(check_trie_error_and_reset() == 0);
   test_assert(hits[0]->nitems == 0);
   test_assert(hits[1]->nitems == 2);
   test_assert(hits[2]->nitems == 4);
   test_assert(hits[3]->nitems == 0);

   reset_gstack(hits);
   search(trie, "ATGCTAGGGTACTCGATAAC", 0, hits, 0, 0);
   test_assert(err == 0);
   test_assert(check_trie_error_and_reset() == 0);
   test_assert(hits[0]->nitems == 1);
   test_assert(hits[1]->nitems == 0);
   test_assert(hits[2]->nitems == 0);
   test_assert(hits[3]->nitems == 0);

   reset_gstack(hits);
   search(trie, " TGCTAGGGTACTCGATAAC", 1, hits, 0, 0);
   test_assert(err == 0);
   test_assert(check_trie_error_and_reset() == 0);
   test_assert(hits[0]->nitems == 0);
   test_assert(hits[1]->nitems == 1);
   test_assert(hits[2]->nitems == 0);
   test_assert(hits[3]->nitems == 0);

   reset_gstack(hits);
   search(trie, "NAAAAAAAAAAAAAAAAAAN", 2, hits, 0, 1);
   test_assert(err == 0);
   test_assert(check_trie_error_and_reset() == 0);
   test_assert(hits[0]->nitems == 0);
   test_assert(hits[1]->nitems == 0);
   test_assert(hits[2]->nitems == 5);
   test_assert(hits[3]->nitems == 0);

   reset_gstack(hits);
   search(trie, "NNAAAAAAAAAAAAAAAANN", 3, hits, 1, 0);
   test_assert(err == 0);
   test_assert(check_trie_error_and_reset() == 0);
   test_assert(hits[0]->nitems == 0);
   test_assert(hits[1]->nitems == 0);
   test_assert(hits[2]->nitems == 0);
   test_assert(hits[3]->nitems == 0);

   reset_gstack(hits);
   search(trie, "AAAAAAAAAANAAAAAAAAA", 0, hits, 0, 0);
   test_assert(err == 0);
   test_assert(check_trie_error_and_reset() == 0);
   test_assert(hits[0]->nitems == 0);
   test_assert(hits[1]->nitems == 0);
   test_assert(hits[2]->nitems == 0);
   test_assert(hits[3]->nitems == 0);

   reset_gstack(hits);
   search(trie, "NNNAGACTTTTCCAGGGTAT", 3, hits, 0, 0);
   test_assert(err == 0);
   test_assert(check_trie_error_and_reset() == 0);
   test_assert(hits[0]->nitems == 0);
   test_assert(hits[1]->nitems == 0);
   test_assert(hits[2]->nitems == 0);
   test_assert(hits[3]->nitems == 1);

   reset_gstack(hits);
   search(trie, "GGGAGACTTTTCCAGGGNNN", 3, hits, 0, 0);
   test_assert(err == 0);
   test_assert(check_trie_error_and_reset() == 0);
   test_assert(hits[0]->nitems == 0);
   test_assert(hits[1]->nitems == 0);
   test_assert(hits[2]->nitems == 0);
   test_assert(hits[3]->nitems == 1);

   reset_gstack(hits);
   search(trie, "   GGGAGACTTTTCCAGGG", 3, hits, 0, 3);
   test_assert(err == 0);
   test_assert(check_trie_error_and_reset() == 0);
   test_assert(hits[0]->nitems == 0);
   test_assert(hits[1]->nitems == 0);
   test_assert(hits[2]->nitems == 0);
   test_assert(hits[3]->nitems == 1);

   reset_gstack(hits);
   search(trie, "   AGACTTTTCCAGGGTAT", 3, hits, 3, 3);
   test_assert(err == 0);
   test_assert(check_trie_error_and_reset() == 0);
   test_assert(hits[0]->nitems == 0);
   test_assert(hits[1]->nitems == 0);
   test_assert(hits[2]->nitems == 0);
   test_assert(hits[3]->nitems == 1);

   reset_gstack(hits);
   search(trie, "                   N", 1, hits, 3, 19);
   test_assert(err == 0);
   test_assert(check_trie_error_and_reset() == 0);
   test_assert(hits[0]->nitems == 0);
   test_assert(hits[1]->nitems == 3);
   test_assert(hits[2]->nitems == 0);
   test_assert(hits[3]->nitems == 0);

   reset_gstack(hits);
   search(trie, "                    ", 1, hits, 19, 0);
   test_assert(err == 0);
   test_assert(check_trie_error_and_reset() == 0);
   test_assert(hits[0]->nitems == 1);
   test_assert(hits[1]->nitems == 2);
   test_assert(hits[2]->nitems == 0);
   test_assert(hits[3]->nitems == 0);

   // Caution here: the hit is present but the initial
   // search conditions are wrong.
   reset_gstack(hits);
   search(trie, "ATGCTAGGGTACTCGATAAC", 0, hits, 20, 1);
   test_assert(err == 0);
   test_assert(check_trie_error_and_reset() == 0);
   test_assert(hits[0]->nitems == 0);
   test_assert(hits[1]->nitems == 0);
   test_assert(hits[2]->nitems == 0);
   test_assert(hits[3]->nitems == 0);

   // Repeat first test cases.
   reset_gstack(hits);
   search(trie, "AAAAAAAAAAAAAAAAAAAA", 3, hits, 0, 18);
   test_assert(err == 0);
   test_assert(check_trie_error_and_reset() == 0);
   test_assert(hits[0]->nitems == 1);
   test_assert(hits[1]->nitems == 4);
   test_assert(hits[2]->nitems == 1);
   test_assert(hits[3]->nitems == 0);

   reset_gstack(hits);
   search(trie, "AAAAAAAAAAAAAAAAAATA", 3, hits, 18, 18);
   test_assert(err == 0);
   test_assert(check_trie_error_and_reset() == 0);
   test_assert(hits[0]->nitems == 0);
   test_assert(hits[1]->nitems == 2);
   test_assert(hits[2]->nitems == 3);
   test_assert(hits[3]->nitems == 1);

   destroy_tower(hits);

}


void
test_mem_1
(void)
{

   int err;

   // Test hstack dynamic growth. Build a trie with 1000 sequences
   // and search with maxdist of 20 (should return all the sequences).
   char seq[9];
   seq[8] = '\0';

   trie_t *trie = new_trie(8);
   test_assert_critical(trie != NULL);

   gstack_t **hits = new_tower(9);
   test_assert_critical(hits != NULL);
 
   // Insert 2049 random sequences.
   for (int i = 0 ; i < 2049 ; i++) {
      for (int j = 0 ; j < 8 ; j++) {
         seq[j] = untranslate[(int)(5 * drand48())];
      }
      void **data = insert_string(trie, seq);
      test_assert(data != NULL);
      *data = data;
   }

   // Search with failure in 'malloc()'.
   set_alloc_failure_rate_to(1);
   redirect_stderr();
   err = search(trie, "NNNNNNNN", 8, hits, 0, 8);
   unredirect_stderr();
   reset_alloc();

   test_assert(err > 0);
   test_assert(hits[8]->nitems > hits[8]->nslots);

   reset_gstack(hits);

   // Do it again without failure.
   err = search(trie, "NNNNNNNN", 8, hits, 0, 8);
   test_assert(err == 0);

   destroy_trie(trie, DESTROY_NODES_YES, NULL);
   destroy_tower(hits);
   
}


void
test_mem_2
(void)
{
   char seq[21];
   seq[20] = '\0';

   // Construct 1000 tries with a 'malloc()' failure rate of 1%.
   // If a segmentation fault happens the test will fail.
   set_alloc_failure_rate_to(0.01);

   redirect_stderr();
   for (int i = 0 ; i < 1000 ; i++) {
      trie_t *trie = new_trie(20);
      if (trie == NULL) {
         // Make sure errors are reported.
         test_assert(check_trie_error_and_reset() > 0);
         continue;
      }
      for (int j = 0 ; j < 100 ; j++) {
         for (int k = 0 ; k < 20 ; k++) {
            seq[k] = untranslate[(int)(5 * drand48())];
         }
         void **data = insert_string(trie, seq);
         if (data == NULL) {
            // Make sure errors are reported.
            test_assert(check_trie_error_and_reset() > 0);
         }
         else {
            test_assert(*data == NULL);
            *data = data;
         }
      }
      destroy_trie(trie, DESTROY_NODES_YES, NULL);
   }

   unredirect_stderr();
   reset_alloc();
   
}


void
test_mem_3
(void)
{

   node_t *root = new_trienode();
   test_assert_critical(root != NULL);

   node_t *nodes = malloc(2000 * sizeof(node_t));
   if (nodes == NULL) {
      fprintf(stderr, "unittest error: %s:%d\n", __FILE__, __LINE__);
      exit(EXIT_FAILURE);
   }

   char seq[21];
   seq[20] = '\0';

   // Construct 1000 tries with a 'malloc()' failure rate of 1%.
   // If a segmentation fault happens the test will fail.
   set_alloc_failure_rate_to(0.01);

   redirect_stderr();
   for (int i = 0 ; i < 1000 ; i++) {
      trie_t *trie = new_trie(20);
      if (trie == NULL) {
         // Make sure errors are reported.
         test_assert(check_trie_error_and_reset() > 0);
         continue;
      }
      node_t *pos = nodes;
      for (int j = 0 ; j < 100 ; j++) {
         for (int k = 0 ; k < 20 ; k++) {
            seq[k] = untranslate[(int)(5 * drand48())];
         }
         void **data = insert_string_wo_malloc(trie, seq, &pos);
         if (data == NULL) {
            // Make sure errors are reported.
            test_assert(check_trie_error_and_reset() > 0);
         }
         else {
            test_assert(*data == NULL);
            *data = data;
         }
      }
      destroy_trie(trie, DESTROY_NODES_NO, NULL);
   }

   unredirect_stderr();
   reset_alloc();
   free(nodes);
   
}


void
test_mem_4
(void)
{

   int err;

   trie_t * trie = setup();
   gstack_t **hits = new_tower(4);
   test_assert_critical(hits != NULL);

   // Test 'search()' 1000 times in a perfectly built trie
   // with a 'malloc()' failure rate of 1%.
   set_alloc_failure_rate_to(0.01);

   redirect_stderr();
   for (int i = 0 ; i < 1000 ; i++) {
      // The following search should not make any call to
      // 'malloc()' and so should have an error code of 0
      // on every call.
      reset_gstack(hits);
      err = search(trie, "AAAAAAAAAAAAAAAAAAAA", 3, hits, 0, 20);
      test_assert(err ==  0);
      test_assert(check_trie_error_and_reset() == 0);
      test_assert(hits[0]->nitems == 1);
      test_assert(hits[1]->nitems == 4);
      test_assert(hits[2]->nitems == 1);
      test_assert(hits[3]->nitems == 0);
   }

   unredirect_stderr();
   reset_alloc();
   teardown(trie);

}


void
test_mem_5
(void)
{

   // The variable 'trie' and 'hstack' are initialized
   // without 'malloc()' failure.
   trie_t *trie = new_trie(20);
   test_assert_critical(trie != NULL);

   gstack_t **hits = new_tower(4);
   test_assert_critical(hits != NULL);

   char seq[21];
   seq[20] = '\0';

   // Build an imperfect trie.
   for (int i = 0 ; i < 2048 ; i++) {
      for (int j = 0 ; j < 20 ; j++) {
         seq[j] = untranslate[(int)(5 * drand48())];
      }
      void **data = insert_string(trie, seq);
      if (data == NULL) {
         test_assert(check_trie_error_and_reset() >  0);
      }
      else {
         *data = data;
      }
   }

   // Test 'search()' in the imperfectly built trie.
   for (int i = 0 ; i < 1000 ; i++) {
      for (int j = 0 ; j < 20 ; j++) {
         seq[j] = untranslate[(int)(5 * drand48())];
      }
      reset_gstack(hits);
      search(trie, seq, 3, hits, 0, 20);
   }

   unredirect_stderr();
   reset_alloc();
   destroy_tower(hits);
   
}


void
test_mem_6
(void)
{

   trie_t *trie;
   node_t *trienode;
   gstack_t *gstack;
   gstack_t **tower;

   // Test constructors with a 'malloc()' failure rate of 1%.
   set_alloc_failure_rate_to(0.01);
   redirect_stderr();

   // Test 'new_trie()'.
   for (int i = 0 ; i < 1000 ; i++) {
      trie = new_trie(20);
      if (trie == NULL) {
         test_assert(check_trie_error_and_reset() >  0);
      }
      else {
         destroy_trie(trie, DESTROY_NODES_YES, NULL);
         trie = NULL;
      }
   }

   // Test 'new_trienode()'.
   for (int i = 0 ; i < 1000 ; i++) {
      trienode = new_trienode();
      if (trienode == NULL) {
         test_assert(check_trie_error_and_reset() >  0);
      }
      else {
         free(trienode);
         trie = NULL;
      }
   }

   // Test 'new_gstack()'.
   for (int i = 0 ; i < 1000 ; i++) {
      gstack = new_gstack();
      if (gstack == NULL) {
         test_assert(check_trie_error_and_reset() >  0);
      }
      else {
         free(gstack);
         gstack = NULL;
      }
   }

   // Test 'new_tower()'.
   for (int i = 0 ; i < 1000 ; i++) {
      tower = new_tower(3);
      if (tower == NULL) {
         test_assert(check_trie_error_and_reset() >  0);
      }
      else {
         destroy_tower(tower);
         tower = NULL;
      }
   }

   unredirect_stderr();
   reset_alloc();

}

void
test_starcode_1
// Basic tests of useq.
(void)
{
   useq_t *case_1 = new_useq(1, "some sequence");
   test_assert_critical(case_1 != NULL);
   test_assert(case_1->count == 1);
   test_assert(strcmp(case_1->seq, "some sequence") == 0);
   test_assert(case_1->matches == NULL);
   test_assert(case_1->canonical == NULL);

   useq_t *case_2 = new_useq(-1, "");
   test_assert_critical(case_2 != NULL);
   test_assert(case_2->count == -1);
   test_assert(strcmp(case_2->seq, "") == 0);
   test_assert(case_2->matches == NULL);
   test_assert(case_2->canonical == NULL);

   addmatch(case_2, case_1, 1, 2);

   test_assert(case_1->matches == NULL);
   test_assert(case_2->matches != NULL);

   destroy_useq(case_1);
   destroy_useq(case_2);

}


void
test_starcode_2
(void)
// Test 'seqsort()' and 'size_order()'.
{

   // Case 1 (no repeat).
   char *sequences_1[10] = {
      "IRrLv<'*3S?UU<JF4S<,", "tcKvz5JTm!h*X0mSTg",
      "tW:0K&Mvtax<PP/qY6er", "hcU+f!=`.Xs6[a,C7XpN",
      ":3ILp'w?)f]4(a;mf%A9", "RlEF',$6[}ouJQyWqqT#",
      "U Ct`3w8(#KAE+z;vh,",  "[S^jXvNS VP' cwg~_iq",
      ".*/@*Q/]]}32kNB#`qqv", "#`Hwp(&,z|bN~07CSID'",
   };
   const char *sorted_1[10] = {
      "tcKvz5JTm!h*X0mSTg",   "U Ct`3w8(#KAE+z;vh,",
      "#`Hwp(&,z|bN~07CSID'", ".*/@*Q/]]}32kNB#`qqv",
      ":3ILp'w?)f]4(a;mf%A9", "IRrLv<'*3S?UU<JF4S<,",
      "RlEF',$6[}ouJQyWqqT#", "[S^jXvNS VP' cwg~_iq",
      "hcU+f!=`.Xs6[a,C7XpN", "tW:0K&Mvtax<PP/qY6er", 
   };

   useq_t *to_sort_1[10];
   for (int i = 0 ; i < 10 ; i++) {
      to_sort_1[i] = new_useq(1, sequences_1[i]);
   }

   seqsort((void **) to_sort_1, 10, size_order, 1);
   for (int i = 0 ; i < 10 ; i++) {
      test_assert(strcmp(to_sort_1[i]->seq, sorted_1[i]) == 0);
      test_assert(to_sort_1[i]->count == 1);
      destroy_useq(to_sort_1[i]);
   }

   // Case 2 (different lengths).
   char *sequences_2[10] = {
      "IRr",                  "tcKvz5JTm!h*X0mSTg",
      "tW:0K&Mvtax<PP/qY6er", "hcU+f!=`.Xs6[a,C7XpNwoi~OWe88",
      "z3ILp'w?)f]4(a;mf9",   "RlEFWqqT#",
      "U Ct`3w8(#Kz;vh,",     "aS^jXvNS VP' cwg~_iq",
      ".*/@*Q/]]}32#`",       "(&,z|bN~07CSID'",
   };
   const char *sorted_2[10] = {
      "IRr",                  "RlEFWqqT#",
      ".*/@*Q/]]}32#`",       "(&,z|bN~07CSID'",
      "U Ct`3w8(#Kz;vh,",     "tcKvz5JTm!h*X0mSTg",
      "z3ILp'w?)f]4(a;mf9",   "aS^jXvNS VP' cwg~_iq",
      "tW:0K&Mvtax<PP/qY6er", "hcU+f!=`.Xs6[a,C7XpNwoi~OWe88",
   };

   useq_t *to_sort_2[10];
   for (int i = 0 ; i < 10 ; i++) {
      to_sort_2[i] = new_useq(1, sequences_2[i]);
   }

   seqsort((void **) to_sort_2, 10, size_order, 1);
   for (int i = 0 ; i < 10 ; i++) {
      test_assert(strcmp(to_sort_2[i]->seq, sorted_2[i]) == 0);
      test_assert(to_sort_2[i]->count == 1);
      destroy_useq(to_sort_2[i]);
   }

   // Case 3 (repeats).
   char *sequences_3[6] = {
      "repeated", "repeated", "repeated", "repeated", "repeated", "xyz"
   };
   char *sorted_3[6] = {
      "xyz", "repeated", NULL, NULL, NULL, NULL,
   };
   int counts[2] = {1,5};

   useq_t *to_sort_3[6];
   for (int i = 0 ; i < 6 ; i++) {
      to_sort_3[i] = new_useq(1, sequences_3[i]);
   }

   seqsort((void **) to_sort_3, 6, size_order, 1);
   for (int i = 0 ; i < 2 ; i++) {
      test_assert(strcmp(to_sort_3[i]->seq, sorted_3[i]) == 0);
      test_assert(to_sort_3[i]->count == counts[i]);
      destroy_useq(to_sort_3[i]);
   }
   for (int i = 2 ; i < 6 ; i++) {
      test_assert(to_sort_3[i] == NULL);
   }

}


void
test_starcode_3
(void)
// Test 'addmatch', 'transfer_counts_and_update_canonicals'.
{
   useq_t *u1 = new_useq(1, "B}d2)$ChPyDC=xZ D-C");
   useq_t *u2 = new_useq(2, "RCD67vQc80:~@FV`?o%D");

   // Add match to 'u1'.
   addmatch(u1, u2, 1, 1);

   test_assert(u1->count == 1);
   test_assert(u2->count == 2);

   // This should not transfer counts but update canonical.
   transfer_counts_and_update_canonicals(u2);

   test_assert(u1->count == 1);
   test_assert(u2->count == 2);
   test_assert(u1->canonical == NULL);
   test_assert(u2->canonical == u2);

   // This should transfer the counts from 'u1' to 'u2'.
   transfer_counts_and_update_canonicals(u1);

   test_assert(u1->count == 0);
   test_assert(u2->count == 3);
   test_assert(u1->canonical == u2);
   test_assert(u2->canonical == u2);

   destroy_useq(u1);
   destroy_useq(u2);

   useq_t *u3 = new_useq(1, "{Lu[T}FOCMs}L_zx");
   useq_t *u4 = new_useq(2, "|kAV|Ch|RZ]h~WjCoDpX");
   useq_t *u5 = new_useq(2, "}lzHolUky");

   // Add matches to 'u3'.
   addmatch(u3, u4, 1, 1);
   addmatch(u3, u5, 1, 1);

   test_assert(u3->count == 1);
   test_assert(u4->count == 2);
   test_assert(u5->count == 2);

   transfer_counts_and_update_canonicals(u3);

   test_assert(u3->count == 0);
   test_assert(u3->count == 0);
   test_assert(u3->canonical == NULL);
   test_assert(u4->canonical == u4);
   test_assert(u5->canonical == u5);

   destroy_useq(u3);
   destroy_useq(u4);
   destroy_useq(u5);

}


void
test_starcode_4
(void)
// Test 'canonical_order'.
{
   useq_t *u1 = new_useq(1, "ABCD");
   useq_t *u2 = new_useq(2, "EFGH");
   test_assert_critical(u1 != NULL);
   test_assert_critical(u2 != NULL);

   test_assert(canonical_order(u1, u2) == -4);
   test_assert(canonical_order(u2, u1) == 4);

   // Add match to 'u1', and update canonicals.
   addmatch(u1, u2, 1, 1);
   transfer_counts_and_update_canonicals(u1);

   test_assert(canonical_order(u1, u2) == -4);
   test_assert(canonical_order(u2, u1) == 4);

   useq_t *u3 = new_useq(1, "CDEF");
   useq_t *u4 = new_useq(2, "GHIJ");
   test_assert_critical(u3 != NULL);
   test_assert_critical(u4 != NULL);

   test_assert(canonical_order(u1, u3) == -1);
   test_assert(canonical_order(u3, u1) == 1);
   test_assert(canonical_order(u1, u4) == -1);
   test_assert(canonical_order(u4, u1) == 1);
   test_assert(canonical_order(u2, u3) == -1);
   test_assert(canonical_order(u3, u2) == 1);
   test_assert(canonical_order(u2, u4) == -1);
   test_assert(canonical_order(u4, u2) == 1);
   test_assert(canonical_order(u3, u4) == -4);
   test_assert(canonical_order(u4, u3) == 4);

   // Add match to 'u3', and update canonicals.
   addmatch(u3, u4, 1, 1);
   transfer_counts_and_update_canonicals(u3);

   test_assert(canonical_order(u1, u3) == -2);
   test_assert(canonical_order(u3, u1) == 2);
   test_assert(canonical_order(u1, u4) == -2);
   test_assert(canonical_order(u4, u1) == 2);
   test_assert(canonical_order(u2, u3) == -2);
   test_assert(canonical_order(u3, u2) == 2);
   test_assert(canonical_order(u2, u4) == -2);
   test_assert(canonical_order(u4, u2) == 2);
   test_assert(canonical_order(u3, u4) == -4);
   test_assert(canonical_order(u4, u3) == 4);

   useq_t *u5 = new_useq(1, "CDEF");
   useq_t *u6 = new_useq(3, "GHIJ");
   test_assert_critical(u5 != NULL);
   test_assert_critical(u6 != NULL);

   test_assert(canonical_order(u1, u5) == -1);
   test_assert(canonical_order(u5, u1) == 1);
   test_assert(canonical_order(u1, u6) == -1);
   test_assert(canonical_order(u6, u1) == 1);
   test_assert(canonical_order(u2, u5) == -1);
   test_assert(canonical_order(u5, u2) == 1);
   test_assert(canonical_order(u2, u6) == -1);
   test_assert(canonical_order(u6, u2) == 1);
   test_assert(canonical_order(u3, u5) == -1);
   test_assert(canonical_order(u5, u3) == 1);
   test_assert(canonical_order(u3, u6) == -1);
   test_assert(canonical_order(u6, u3) == 1);
   test_assert(canonical_order(u4, u5) == -1);
   test_assert(canonical_order(u5, u4) == 1);
   test_assert(canonical_order(u4, u6) == -1);
   test_assert(canonical_order(u6, u4) == 1);
   test_assert(canonical_order(u5, u6) == -4);
   test_assert(canonical_order(u6, u5) == 4);

   // Add match to 'u5', and update canonicals.
   addmatch(u5, u6, 1, 1);
   transfer_counts_and_update_canonicals(u5);

   test_assert(canonical_order(u1, u5) == 1);
   test_assert(canonical_order(u5, u1) == -1);
   test_assert(canonical_order(u1, u6) == 1);
   test_assert(canonical_order(u6, u1) == -1);
   test_assert(canonical_order(u2, u5) == 1);
   test_assert(canonical_order(u5, u2) == -1);
   test_assert(canonical_order(u2, u6) == 1);
   test_assert(canonical_order(u6, u2) == -1);
   test_assert(canonical_order(u3, u5) == 1);
   test_assert(canonical_order(u5, u3) == -1);
   test_assert(canonical_order(u3, u6) == 1);
   test_assert(canonical_order(u6, u3) == -1);
   test_assert(canonical_order(u4, u5) == 1);
   test_assert(canonical_order(u5, u4) == -1);
   test_assert(canonical_order(u4, u6) == 1);
   test_assert(canonical_order(u6, u4) == -1);
   test_assert(canonical_order(u5, u6) == -4);
   test_assert(canonical_order(u6, u5) == 4);

   destroy_useq(u1);
   destroy_useq(u2);
   destroy_useq(u3);
   destroy_useq(u4);
   destroy_useq(u5);
   destroy_useq(u6);

}


void
test_starcode_5
(void)
// Test 'count_order()' and 'seqsort()'.
{

   useq_t *u1 = new_useq(1, "L@[ohztp{2@V(u(x7fLt&x80");
   useq_t *u2 = new_useq(2, "$Ee6xkB+.Q;Nk)|w[KQ;");
   test_assert(count_order(u1, u2) == 1);
   test_assert(count_order(u2, u1) == -1);
   test_assert(count_order(u1, u1) == 0);
   test_assert(count_order(u2, u2) == 0);

   destroy_useq(u1);
   destroy_useq(u2);

   for (int i = 0 ; i < 1000 ; i++) {
      char seq1[21] = {0};
      char seq2[21] = {0};
      for (int j = 0 ; j < 20 ; j++) {
         seq1[j] = untranslate[(int)(5 * drand48())];
         seq2[j] = untranslate[(int)(5 * drand48())];
      }
      int randint = (int)(4096 * drand48());
      u1 = new_useq(randint, seq1);
      u2 = new_useq(randint + 1, seq2);
      test_assert(count_order(u1, u2) == 1);
      test_assert(count_order(u2, u1) == -1);
      test_assert(count_order(u1, u1) == 0);
      test_assert(count_order(u2, u2) == 0);

      destroy_useq(u1);
      destroy_useq(u2);
   }

   // Case 1 (no repeat).
   char *sequences_1[10] = {
      "IRrLv<'*3S?UU<JF4S<,", "tcKvz5JTm!h*X0mSTg",
      "tW:0K&Mvtax<PP/qY6er", "hcU+f!=`.Xs6[a,C7XpN",
      ":3ILp'w?)f]4(a;mf%A9", "RlEF',$6[}ouJQyWqqT#",
      "U Ct`3w8(#KAE+z;vh,",  "[S^jXvNS VP' cwg~_iq",
      ".*/@*Q/]]}32kNB#`qqv", "#`Hwp(&,z|bN~07CSID'",
   };
   const char *sorted_1[10] = {
      "#`Hwp(&,z|bN~07CSID'", ".*/@*Q/]]}32kNB#`qqv", 
      "[S^jXvNS VP' cwg~_iq", "U Ct`3w8(#KAE+z;vh,", 
      "RlEF',$6[}ouJQyWqqT#", ":3ILp'w?)f]4(a;mf%A9", 
      "hcU+f!=`.Xs6[a,C7XpN", "tW:0K&Mvtax<PP/qY6er", 
      "tcKvz5JTm!h*X0mSTg",   "IRrLv<'*3S?UU<JF4S<,", 
   };

   useq_t *to_sort_1[10];
   for (int i = 0 ; i < 10 ; i++) {
      to_sort_1[i] = new_useq(i, sequences_1[i]);
   }

   seqsort((void **) to_sort_1, 10, count_order, 1);
   for (int i = 0 ; i < 10 ; i++) {
      test_assert(strcmp(to_sort_1[i]->seq, sorted_1[i]) == 0);
      test_assert(to_sort_1[i]->count == 9-i);
      destroy_useq(to_sort_1[i]);
   }

   // Case 2 (repeats).
   char *sequences_2[6] = {
      "repeated", "repeated", "repeated", "abc", "abc", "xyz"
   };
   int counts[6] = {1,1,2,3,4,4};
   char *sorted_2[6] = {
      "abc", "xyz", "abc", "repeated", "repeated", NULL,
   };
   int sorted_counts[5] = {4,4,3,2,1};

   useq_t *to_sort_2[6];
   for (int i = 0 ; i < 6 ; i++) {
      to_sort_2[i] = new_useq(counts[i], sequences_2[i]);
   }

   seqsort((void **) to_sort_2, 6, count_order, 1);
   for (int i = 0 ; i < 5 ; i++) {
      test_assert(strcmp(to_sort_2[i]->seq, sorted_2[i]) == 0);
      test_assert(to_sort_2[i]->count == sorted_counts[i]);
      destroy_useq(to_sort_2[i]);
   }
   for (int i = 5 ; i < 6 ; i++) {
      test_assert(to_sort_2[i] == NULL);
   }

}


void
test_starcode_6
(void)
// Test 'pad_useq()' and 'unpad_useq()'
{

   gstack_t * useqS = new_gstack();
   test_assert_critical(useqS != NULL);

   useq_t *u1 = new_useq(1, "L@[ohztp{2@V(u(x7fLt&x80");
   useq_t *u2 = new_useq(2, "$Ee6xkB+.Q;Nk)|w[KQ;");
   test_assert_critical(u1 != NULL);
   test_assert_critical(u2 != NULL);

   push(u1, &useqS);
   push(u2, &useqS);
   test_assert(useqS->nitems == 2);

   int med;
   pad_useq(useqS, &med);
   test_assert(strcmp(u2->seq, "    $Ee6xkB+.Q;Nk)|w[KQ;") == 0);
   test_assert(med == 20);

   useq_t *u3 = new_useq(23, "0sdfd:'!'@{1$Ee6xkB+.Q;[Nk)|w[KQ;");
   test_assert_critical(u3 != NULL);
   push(u3, &useqS);
   test_assert(useqS->nitems == 3);

   pad_useq(useqS, &med);
   test_assert(strcmp(u1->seq, "         L@[ohztp{2@V(u(x7fLt&x80") == 0);
   test_assert(strcmp(u2->seq, "             $Ee6xkB+.Q;Nk)|w[KQ;") == 0);
   test_assert(med == 24);

   unpad_useq(useqS);
   test_assert(strcmp(u1->seq, "L@[ohztp{2@V(u(x7fLt&x80") == 0);
   test_assert(strcmp(u2->seq, "$Ee6xkB+.Q;Nk)|w[KQ;") == 0);
   test_assert(strcmp(u3->seq, "0sdfd:'!'@{1$Ee6xkB+.Q;[Nk)|w[KQ;") == 0);

   destroy_useq(u1);
   destroy_useq(u2);
   destroy_useq(u3);
   free(useqS);

}


void
test_starcode_7
(void)
// Test 'new_lookup()'
{

   lookup_t * lut = new_lookup(20, 20, 3, 3);
   test_assert_critical(lut != NULL);
   test_assert(lut->kmers == 3+1);
   test_assert(lut->offset == 3-3);
   test_assert_critical(lut->klen != NULL);
   int klen[4] = {3,3,3,3};
   for (int i = 0 ; i < 4 ; i++) {
      test_assert(lut->klen[i] == klen[i]);
      free(lut->lut[i]);
   }

   free(lut);

}


int
main(
   int argc,
   char **argv
)
{

   // Register test cases //
   const static test_case_t test_cases[] = {
      {"trie/base/1", test_base_1},
      {"trie/base/2", test_base_2},
      {"trie/base/3", test_base_3},
      {"trie/base/4", test_base_4},
      {"trie/base/5", test_base_5},
      {"trie/base/6", test_base_6},
      {"trie/base/7", test_base_7},
      {"trie/base/8", test_base_8},
      {"errmsg", test_errmsg},
      {"search", test_search},
      {"mem/1", test_mem_1},
      {"mem/2", test_mem_2},
      {"mem/3", test_mem_3},
      {"mem/4", test_mem_4},
      {"mem/5", test_mem_5},
      {"mem/6", test_mem_6},
      {"starcode/base/1", test_starcode_1},
      {"starcode/base/2", test_starcode_2},
      {"starcode/base/3", test_starcode_3},
      {"starcode/base/4", test_starcode_4},
      {"starcode/base/5", test_starcode_5},
      {"starcode/base/6", test_starcode_6},
      {"starcode/base/7", test_starcode_7},
      {NULL, NULL}
   };

   return run_unittest(argc, argv, test_cases);

}