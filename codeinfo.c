/*
 * $Id: codeinfo.c,v 1.1.1.1 1996/06/30 23:51:51 grubba Exp $
 *
 * Functions for handling segments of code.
 *
 * $Log: codeinfo.c,v $
 * Revision 1.8  1996/06/30 23:06:31  grubba
 * Fixed several bugs in insert_seg*().
 * Added sanity checks.
 *
 * Revision 1.7  1996/06/19 11:08:25  grubba
 * Some minor bugs fixed.
 *
 * Revision 1.6  1996/05/11 17:22:20  grubba
 * destroy_seg added and it seems to work!
 * parent pointer added to the seg_info to simplify deletion, it's hard enough anyway.
 *
 * Revision 1.5  1996/05/09 09:45:06  grubba
 * Some more optimizations.
 * Added some more sanity checks.
 * Made the sanity checks optional (NDEBUG).
 *
 * Revision 1.4  1996/05/08 21:45:35  grubba
 * Some more optimizations.
 *
 * Revision 1.3  1996/05/08 21:24:34  grubba
 * Now seems to work.
 *
 * Revision 1.2  1996/05/08 07:49:15  grubba
 * Optimized code size for find_seg() a bit.
 *
 * Revision 1.1  1996/05/08 07:24:22  grubba
 * Initial revision
 *
 */

/*
 * Includes
 */

#include <setjmp.h>
#include <stdlib.h>
#include <stdio.h>

#include "recomp.h"
#include "m68k.h"
#include "codeinfo.h"

/*
 * Debug
 */

#define DPRINTF(x)

#ifndef DPRINTF
#ifdef DEBUG
#define DPRINTF(x)	printf x
#else
#define DPRINTF(x)
#endif /* DEBUG */
#endif /* !DPRINTF */

/*
 * Globals
 */

struct seg_info *code_tree;

/*
 * Prototypes
 */

static inline struct seg_info *insert_seg1(struct seg_info **root, ULONG maddr, ULONG mend, jmp_buf back);
static struct seg_info *insert_seg_left(struct seg_info **root, ULONG maddr, ULONG mend, jmp_buf back);
static struct seg_info *insert_seg_right(struct seg_info **root, ULONG maddr, ULONG mend, jmp_buf back);
static void destroy_fix_seg(struct seg_info **root, struct seg_info *seg);
static void free_seg(struct seg_info *seg);
static void free_codeinfo(struct code_info *ci);

/*
 * Functions
 */

/*
 * Segment handling
 */
struct seg_info *find_seg(struct seg_info *root, ULONG maddr)
{
  if ((!root) || (maddr >= root->mmax)) {
    return (NULL);
  }

find_right:
  /* mmax already checked */
  if (maddr < root->maddr) {
    if (maddr < (root->mmin & ~SEG_COLOR_RED)) {
      return (NULL);
    } else {
      root = root->left;
      goto find_left;
    }
  } else if (maddr < root->mend) {
    return (root);
  } else {
    root = root->right;
    goto find_right;
  }

  /* NOT_REACHED */

find_left:
  /* mmin is already checked */
  if (maddr < root->mend) {
    if (maddr < root->maddr) {
      root = root->left;
      goto find_left;
    } else {
      return (root);
    }
  } else if (maddr < root->mmax) {
    root = root->right;
    goto find_right;
  } else {
    return (NULL);
  }
  /* NOT_REACHED */
}

int check_segtree_balance(struct seg_info *root, ULONG min, ULONG max, ULONG color)
{
  if (!root) {
    return(0);
  } else {

    int ldepth = 0;
    int rdepth = 0;

    if (root->mmin & color) {
      fprintf(stderr, "SANITY: Bad tree coloring!\n");
      dump_seg_tree(root);
      dump_seg_tree(code_tree);
    }

    if ((root->mend > max) || (root->maddr < min)) {
      fprintf(stderr, "SANITY: Tree overlap 0x%08lx, 0x%08lx <=> 0x%08lx, 0x%08lx\n",
	      root->maddr, root->mend, min, max);
      dump_seg_tree(root);
      dump_seg_tree(code_tree);
    }

    if (root->left) {
      ldepth = check_segtree_balance(root->left, root->mmin & ~SEG_COLOR_RED, root->maddr, root->mmin & SEG_COLOR_RED);
    }
    if (root->right) {
      rdepth = check_segtree_balance(root->right, root->mend, root->mmax, root->mmin & SEG_COLOR_RED);
    }

    if (ldepth != rdepth) {
      fprintf(stderr, "SANITY: Unbalanced tree!\n");
      dump_seg_tree(root);
      if (ldepth > rdepth) {
	rdepth = ldepth;
      } else {
	ldepth = rdepth;
      }
    }

    if (!root->mmin & SEG_COLOR_RED) {
      return(ldepth + 1);
    } else {
      return(ldepth);
    }
  }
}

void check_segtree_sanity(struct seg_info *root)
{
  (void)check_segtree_balance(root, 0, ~0, SEG_COLOR_RED);
}

struct seg_info *insert_seg(struct seg_info **root, ULONG maddr, ULONG mend)
{
  struct seg_info *ret_val;
  jmp_buf fastreturn;

  if (!(ret_val = (struct seg_info *)setjmp(fastreturn))) {
    ret_val = insert_seg1(root, maddr, mend, fastreturn);
  } else if (ret_val == (struct seg_info *)1) {
    /* longjmp converts NULL to 1, so we need to convert it back here */
    ret_val = NULL;
  }
  if (*root) {
    /* Color the root black, and propagate the min-value */
    if ((*root)->left) {
      (*root)->mmin = (*root)->left->mmin & ~SEG_COLOR_RED;
    } else {
      (*root)->mmin &= ~SEG_COLOR_RED;
    }
    /* Propagate the max-value */
    if ((*root)->right) {
      (*root)->mmax = (*root)->right->mmax;
    }
  }

#ifdef DEBUG
  check_segtree_sanity(*root);
#endif
  return(ret_val);
}

static inline struct seg_info *insert_seg1(struct seg_info **root, ULONG maddr, ULONG mend, jmp_buf back)
{
  /* NOTE: Inclusive search */
  if (!(*root)) {
    /* Empty tree */
    DPRINTF(("DEBUG: insert_seg1 EMPTY\n"));
    (*root) = new_seg(maddr, mend);
    return (*root);
  } else if (mend <= (*root)->mend) {
    if (mend <= (*root)->maddr) {
      /* In left sub-tree */
      DPRINTF(("DEBUG: insert_seg1 LEFT\n"));
      return(insert_seg_left(root, maddr, mend, back));
#ifndef NDEBUG
    } else if (mend != (*root)->mend) {
      /* Sanity check */
      /* OVERLAP_ERROR */
      fprintf(stderr, "insert_seg1(): Overlap error\n");
      return (NULL);
#endif
    } else {
      /* In root segment */
      if (maddr < (*root)->maddr) {
	/* Expand segment */
	/* We know that the root node is always black
	 * -- no need for filtering
	 */
	DPRINTF(("DEBUG: insert_seg1 EXPAND ROOT\n"));
	if (maddr < (*root)->mmin) {
#ifndef NDEBUG
	  /* Sanity check */
	  if ((*root)->left) {
	    fprintf(stderr, "insert_seg1(): Overlap error\n");
	    return(NULL);
	  }
#endif
	  /* This node is always black */
	  (*root)->mmin = maddr;
	}
	(*root)->maddr = maddr;
      }
      return(*root);
    }
  } else {
    /* In right sub-tree */
    DPRINTF(("DEBUG: insert_seg1 RIGHT\n"));
    return (insert_seg_right(root, maddr, mend, back));
  }
}

static struct seg_info *insert_seg_left(struct seg_info **root, ULONG maddr, ULONG mend, jmp_buf back)
{
  struct seg_info *ret_val;

  /* NOTE: Inclusive search */
  if (!(*root)->left) {
    /* Empty sub-tree */
    DPRINTF(("DEBUG: insert_seg_left EMPTY\n"));
    (*root)->left = new_seg(maddr, mend);
    (*root)->left->parent = (*root);
    return ((*root)->left);
  } else if (mend <= ((*root)->left)->mend) {
    if (mend <= (*root)->left->maddr) {
      /* In left sub-tree */
      DPRINTF(("DEBUG: insert_seg_left LEFT\n"));
      ret_val = insert_seg_left(&((*root)->left), maddr, mend, back);
      if ((*root)->left->mmin & (*root)->left->left->mmin & SEG_COLOR_RED) {
	/* Need to fix colors */
	if (((*root)->right) && ((*root)->right->mmin & SEG_COLOR_RED)) {
	  /* CASE 1 
	   *
	   *     *            O
	   *    / \          / \
	   *   O   O  =>    *   *
	   *  /            /
	   * O            O
	   *
	   */
	  DPRINTF(("DEBUG: insert_seg_left CASE 1\n"));
	  (*root)->mmin |= SEG_COLOR_RED;
	  (*root)->left->mmin = (*root)->left->left->mmin & ~SEG_COLOR_RED;
	  (*root)->right->mmin &= ~SEG_COLOR_RED;
	} else {
	  /* CASE 2 
	   *
	   *     *          *
	   *    / \        / \
	   *   O   *  =>  O   O
	   *  /                \
	   * O                  *
	   *
	   */
	  struct seg_info *new_root = (*root)->left;
	  DPRINTF(("DEBUG: insert_seg_left CASE 2\n"));
	  (*root)->left = new_root->right;
	  if ((*root)->left) {
	    (*root)->left->parent = (*root);
	    (*root)->mmin = SEG_COLOR_RED | (*root)->left->mmin;
	  } else {
	    (*root)->mmin = SEG_COLOR_RED | (*root)->maddr;
	  }
	  new_root->parent = (*root)->parent;
	  new_root->right = (*root);
	  new_root->right->parent = new_root;
	  new_root->mmax = (*root)->mmax;
	  new_root->mmin &= ~SEG_COLOR_RED;
	  *root = new_root;
	  /* Here a fast return would be possible but for mmin on previous nodes */
	}
      } else {
	/* Propagate mmin */
	DPRINTF(("DEBUG: insert_seg_left PROPAGATE\n"));
	(*root)->left->mmin = ((*root)->left->mmin & SEG_COLOR_RED) |
	                      ((*root)->left->left->mmin & ~SEG_COLOR_RED);
      }
      return(ret_val);
#ifndef NDEBUG
    } else if (mend != (*root)->left->mend) {
      /* Sanity check */
      /* OVERLAP_ERROR */
      fprintf(stderr, "insert_seg_left(): Overlap error\n");
      /* Since we don't modify anything we can do a fast return here */
      longjmp(back, (int)NULL);
#endif
    } else {
      /* In existing segment
       *
       *   X
       *  /
       * X
       *
       */
      DPRINTF(("DEBUG: insert_seg_left EXISTING\n"));
      if (maddr < (*root)->left->maddr) {
	/* Expand segment */
	if (maddr < ((*root)->left->mmin & ~SEG_COLOR_RED)) {
#ifndef NDEBUG
	  /* Sanity check */
	  if ((*root)->left->left) {
	    fprintf(stderr, "insert_seg_left(): Overlap error\n");
	    /* Do a fast return */
	    longjmp(back, (int)NULL);
	  }
#endif
	  (*root)->left->maddr = maddr;
	  if ((*root)->left->left) {
	    /* A fast return is possible, since we havent changed color or sub-tree size */

	    longjmp(back, (int)((*root)->left));
	  } else {
	    (*root)->left->mmin = ((*root)->left->mmin & SEG_COLOR_RED) | maddr;
	  }
	  /* Here a fast return would be possible but for mmin on previous nodes */
	  return((*root)->left);
	}
	(*root)->left->maddr = maddr;
      }
      /* The mmin value remains unmodified, thus we can do a fast return */
      longjmp(back, (int)(*root)->left);
    }
  } else {
    /* In right sub-tree */
    DPRINTF(("DEBUG: insert_seg_left RIGHT\n"));
    ret_val = insert_seg_right(&((*root)->left), maddr, mend, back);
    if ((*root)->left->mmin & (*root)->left->right->mmin & SEG_COLOR_RED) {
      /* Need to fix colors */
      if (((*root)->right) && ((*root)->right->mmin & SEG_COLOR_RED)) {
	/* CASE 1
	 *
	 *   *          O
	 *  / \        / \
	 * O   O  =>  *   *
	 *  \          \
	 *   O          O
	 *
	 */
	DPRINTF(("DEBUG: insert_seg_left CASE 1b\n"));
	(*root)->mmin |= SEG_COLOR_RED;
	(*root)->left->mmin &= ~SEG_COLOR_RED;
	(*root)->right->mmin &= ~SEG_COLOR_RED;
	(*root)->left->mmax = (*root)->left->right->mmax;

	/* Fast return not possible here */
	return(ret_val);
      } else {
	/* CASE 3
	 *
	 *   *          *
	 *  / \        / \
	 * O   *  =>  O   O
	 *  \              \
	 *   O              *
	 *
	 */
	struct seg_info *new_root = (*root)->left->right;
	new_root->parent = (*root)->parent;

	DPRINTF(("DEBUG: insert_seg_left CASE 3\n"));
	(*root)->left->right = new_root->left;
	if ((*root)->left->right) {
	  (*root)->left->right->parent = (*root)->left;
	  (*root)->left->mmax = (*root)->left->right->mmax;
	} else {
	  (*root)->left->mmax = (*root)->left->mend;
	}
	new_root->left = (*root)->left;
	new_root->left->parent = new_root;
	new_root->mmin = new_root->left->mmin & ~SEG_COLOR_RED;

	(*root)->left = new_root->right;
	if ((*root)->left) {
	  (*root)->left->parent = (*root);
	  (*root)->mmin = (*root)->left->mmin | SEG_COLOR_RED;
	} else {
	  (*root)->mmin = SEG_COLOR_RED | (*root)->maddr;
	}
	new_root->right = (*root);
	new_root->right->parent = new_root;
	new_root->mmax = (*root)->mmax;

	*root = new_root;
      }
    } else {
      /* Propagate up the new maxvalue */
      DPRINTF(("DEBUG: insert_seg_left PROPAGATE b\n"));
      (*root)->left->mmax = (*root)->left->right->mmax;

      if ((*root)->mmin & (*root)->left->mmin & SEG_COLOR_RED) {
	/* We still need to propagate upwards */
	return(ret_val);
      }
    }

    /* Here a fast return *IS* possible! */
    longjmp(back, (int)ret_val);
  }
  
}

static struct seg_info *insert_seg_right(struct seg_info **root, ULONG maddr, ULONG mend, jmp_buf back)
{
  struct seg_info *ret_val;

  /* NOTE: Inclusive search */
  if (!(*root)->right) {
    /* Empty sub-tree */
    DPRINTF(("DEBUG: insert_seg_right EMPTY\n"));
#ifndef NDEBUG
    /* Sanity check */
    if (maddr < (*root)->mend) {
      fprintf(stderr, "insert_seg_right(): Overlap error\n");
      /* Do a fast return */
      longjmp(back, (int)NULL);
    }
#endif
    (*root)->right = new_seg(maddr, mend);
    (*root)->right->parent = (*root);
    return ((*root)->right);
  } else if (mend <= ((*root)->right)->mend) {
    if (mend <= (*root)->right->maddr) {
      /* In left sub-tree */
      DPRINTF(("DEBUG: insert_seg_right LEFT\n"));
      ret_val = insert_seg_left(&((*root)->right), maddr, mend, back);
      if ((*root)->right->mmin & (*root)->right->left->mmin & SEG_COLOR_RED) {
	/* Need to fix colors */
	if (((*root)->left) && ((*root)->left->mmin & SEG_COLOR_RED)) {
	  /* CASE 1
	   *
	   *   *          O
	   *  / \        / \
	   * O   O  =>  *   *
	   *    /          /
	   *   O          O
	   *
	   */
	  DPRINTF(("DEBUG: insert_seg_right CASE 1\n"));
	  (*root)->mmin |= SEG_COLOR_RED;
	  (*root)->left->mmin &= ~SEG_COLOR_RED;
	  (*root)->right->mmin = (*root)->right->left->mmin & ~SEG_COLOR_RED;

	  /* Fast return not possible here */
	  return(ret_val);
	} else {
	  /* CASE 3
	   *
	   *   *            *
	   *  / \          / \
	   * *   O  =>    O   O
	   *    /        /
	   *   O        *
	   *
	   */
	  struct seg_info *new_root = (*root)->right->left;
	  new_root->parent = (*root)->parent;

	  DPRINTF(("DEBUG: insert_seg_right CASE 3\n"));
	  (*root)->right->left = new_root->right;
	  if ((*root)->right->left) {
	    (*root)->right->left->parent = (*root)->right;
	    (*root)->right->mmin = (*root)->right->left->mmin | SEG_COLOR_RED;
	  } else {
	    (*root)->right->mmin = (*root)->right->maddr | SEG_COLOR_RED;
	  }
	  new_root->right = (*root)->right;
	  new_root->right->parent = new_root;
	  new_root->mmax = new_root->right->mmax;

	  (*root)->right = new_root->left;
	  (*root)->mmin |= SEG_COLOR_RED;
	  if ((*root)->right) {
	    (*root)->right->parent = (*root);
	    (*root)->mmax = (*root)->right->mmax;
	  } else {
	    (*root)->mmax = (*root)->mend;
	  }
	  new_root->left = (*root);
	  new_root->left->parent = new_root;
	  new_root->mmin = (*root)->mmin & ~SEG_COLOR_RED;
	  
	  *root = new_root;
	}
      } else {
	DPRINTF(("DEBUG: insert_seg_right PROPAGATE\n"));
	(*root)->right->mmin = ((*root)->right->mmin & SEG_COLOR_RED) |
	                       ((*root)->right->left->mmin & ~SEG_COLOR_RED);


	if ((*root)->mmin & (*root)->right->mmin & SEG_COLOR_RED) {
	  /* We still need to propagate upwards */
	  return(ret_val);
	}
      }

      /* Here a fast return *IS* possible! */
      longjmp(back, (int)ret_val);
#ifndef NDEBUG
    } else if (mend != (*root)->right->mend) {
      /* Sanity check */
      /* OVERLAP_ERROR */
      fprintf(stderr, "insert_seg_right(): Overlap error\n");
      /* Since we don't modify anything we can do a fast return */
      longjmp(back, NULL);
#endif
    } else {
      /* In existing segment
       *
       * X
       *  \
       *   X
       *
       */
      DPRINTF(("DEBUG: insert_seg_right EXISTING\n"));
      if (maddr < (*root)->right->maddr) {
	/* Expand segment */
	if (maddr < ((*root)->right->mmin & ~SEG_COLOR_RED)) {
#ifndef NDEBUG
	  /* Sanity check */
	  if (((*root)->right->left) || (maddr < (*root)->mend)) {
	    fprintf(stderr, "insert_seg_right(): Overlap error\n");
	    /* Do a fast return */
	    longjmp(back, (int)NULL);
	  }
#endif
	  if (!((*root)->right->left)) {
	    (*root)->right->mmin = ((*root)->right->mmin & SEG_COLOR_RED) | maddr;
	  }
	}
	(*root)->right->maddr = maddr;
      }
      /* Here a fast return *IS* possible, since we are on a right node */
      longjmp(back, (int)(*root)->right);
    }
  } else {
    /* In right sub-tree */
    DPRINTF(("DEBUG: insert_seg_right RIGHT\n"));
    ret_val = insert_seg_right(&((*root)->right), maddr, mend, back);
    if ((*root)->right->mmin & (*root)->right->right->mmin & SEG_COLOR_RED) {
      /* Need to fix colors */
      if (((*root)->left) && ((*root)->left->mmin & SEG_COLOR_RED)) {
	/* CASE 1
	 *
	 *   *            O
	 *  / \          / \
	 * O   O    =>  *   *
	 *      \            \
	 *       O            O
	 *
	 */
	DPRINTF(("DEBUG: insert_seg_right CASE 1b\n"));
	(*root)->right->mmax = (*root)->right->right->mmax;
	(*root)->mmin |= SEG_COLOR_RED;
	(*root)->right->mmin &= ~SEG_COLOR_RED;
	(*root)->left->mmin &= ~SEG_COLOR_RED;
      } else {
	/* CASE 2
	 *
	 *   *              *
	 *  / \            / \
	 * *   O    =>    O   O
	 *      \        /
	 *       O      *
	 *
	 */
	struct seg_info *new_root = (*root)->right;
	DPRINTF(("DEBUG: insert_seg_right CASE 2\n"));
	new_root->parent = (*root)->parent;
	(*root)->right = new_root->left;
	(*root)->mmin |= SEG_COLOR_RED;
	if ((*root)->right) {
	  (*root)->right->parent = (*root);
	  (*root)->mmax = (*root)->right->mmax;
	} else {
	  (*root)->mmax = (*root)->mend;
	}
	new_root->left = (*root);
	new_root->left->parent = new_root;
	new_root->mmin = (*root)->mmin & ~SEG_COLOR_RED;
	*root = new_root;
	/* Here a fast return would be possible but for mmax on previous nodes */
      }
    } else {
      /* Propagate up the new maxvalue */
      DPRINTF(("DEBUG: insert_seg_right PROPAGATE b\n"));
      (*root)->right->mmax = (*root)->right->right->mmax;
    }
    return(ret_val);
  }
}

inline static void rotate_left_seg(struct seg_info **root, struct seg_info *seg)
{
  DPRINTF(("DEBUG: rotate left_seg\n"));
  seg->right->parent = seg->parent;
  seg->parent = seg->right;
  seg->right = seg->parent->left;
  seg->parent->left = seg;
  if (seg->right) {
    seg->right->parent = seg;
    seg->mmax = seg->right->mmax;
  } else {
    seg->mmax = seg->mend;
  }
  seg->parent->mmin = (seg->parent->mmin & SEG_COLOR_RED) |
                      (seg->mmin & ~SEG_COLOR_RED);
  if (seg->parent->parent) {
    if (seg->parent->parent->left == seg) {
      seg->parent->parent->left = seg->parent;
    } else {
      seg->parent->parent->right = seg->parent;
    }
  } else {
    *root = seg->parent;
  }
}

inline static void rotate_right_seg(struct seg_info **root, struct seg_info *seg)
{
  DPRINTF(("DEBUG: rotate_right_seg\n"));
  seg->left->parent = seg->parent;
  seg->parent = seg->left;
  seg->left = seg->parent->right;
  seg->parent->right = seg;
  if (seg->left) {
    seg->left->parent = seg;
    seg->mmin = (seg->mmin & SEG_COLOR_RED) |
                (seg->left->mmin & ~SEG_COLOR_RED);
  } else {
    seg->mmin = (seg->mmin & SEG_COLOR_RED) | seg->maddr;
  }
  seg->parent->mmax = seg->mmax;
  if (seg->parent->parent) {
    if (seg->parent->parent->left == seg) {
      seg->parent->parent->left = seg->parent;
    } else {
      seg->parent->parent->right = seg->parent;
    }
  } else {
    *root = seg->parent;
  }
}

static struct seg_info *min_seg(struct seg_info *root)
{
  if (root) {
    while (root->left) {
      root = root->left;
    }
  }
  return (root);
}

void destroy_seg(struct seg_info **root, struct seg_info *seg)
{
  jmp_buf fastreturn;
  struct seg_info *yseg = seg;
  struct seg_info *xseg;

  if (seg) {
    if (seg->right) {
      if (seg->left) {
	yseg = min_seg(seg->right);
      }
      xseg = yseg->right;
    } else {
      xseg = yseg->left;
    }
    if (yseg != seg) {
      seg->maddr = yseg->maddr;
      seg->mend = yseg->mend;
    }
    if (xseg) {
      xseg->parent = yseg->parent;
      if (xseg->parent) {
	if (xseg->parent->left == yseg) {
	  xseg->parent->left = xseg;
	  xseg->parent->mmin = (xseg->parent->mmin & SEG_COLOR_RED) |
	                       (xseg->mmin & ~SEG_COLOR_RED);
	} else {
	  xseg->parent->right = xseg;
	  xseg->parent->mmax = xseg->mmax;
	}
	if (!(yseg->mmin & SEG_COLOR_RED)) {
	  destroy_fix_seg(root, xseg);
	}
      } else {
	*root = xseg;
      }
    } else {
      /* First xseg is NULL */
      struct seg_info *parent = yseg->parent;
      if (parent) {
	if (!(yseg->mmin & SEG_COLOR_RED)) {
	  struct seg_info *wseg;
	  if (parent->left == yseg) {
	    parent->left = NULL;
	    parent->mmin = (parent->mmin & SEG_COLOR_RED) | parent->maddr;
	    wseg = parent->right;
	    if (wseg->mmin & SEG_COLOR_RED) {
	      /* CASE 1 */
	      wseg->mmin &= ~SEG_COLOR_RED;
	      parent->mmin |= SEG_COLOR_RED;
	      /* Rotate left */
	      rotate_left_seg(root, parent);
	      wseg = parent->right;
	    }
	    if (((!wseg->left) || (!(wseg->left->mmin & SEG_COLOR_RED))) &&
		((!wseg->right) || (!(wseg->right->mmin & SEG_COLOR_RED)))) {
	      /* CASE 2 */
	      wseg->mmin |= SEG_COLOR_RED;
	      destroy_fix_seg(root, parent);
	    } else {
	      if ((!wseg->right) || (!(wseg->right->mmin & SEG_COLOR_RED))) {
		/* CASE 3 */
		wseg->left->mmin &= ~SEG_COLOR_RED;
		wseg->mmin |= SEG_COLOR_RED;
		rotate_right_seg(root, wseg);
		wseg = parent->right;
	      }
	      /* CASE 4 */
	      wseg->mmin = (parent->mmin & SEG_COLOR_RED) |
		           (wseg->mmin & ~SEG_COLOR_RED);
	      parent->mmin &= ~SEG_COLOR_RED;
	      if (wseg->right) {
		wseg->right->mmin &= ~SEG_COLOR_RED;
	      }
	      rotate_left_seg(root, parent);
	    }
	  } else {
	    parent->right = NULL;
	    parent->mmax = parent->mend;
	    wseg = parent->left;
	    if (wseg->mmin & SEG_COLOR_RED) {
	      /* CASE 1 */
	      wseg->mmin &= ~SEG_COLOR_RED;
	      parent->mmin |= SEG_COLOR_RED;
	      /* Rotate right */
	      rotate_right_seg(root, parent);
	      wseg = parent->left;
	    }
	    if (((!wseg->left) || (!(wseg->left->mmin & SEG_COLOR_RED))) &&
		((!wseg->right) || (!(wseg->right->mmin & SEG_COLOR_RED)))) {
	      /* CASE 2 */
	      wseg->mmin |= SEG_COLOR_RED;
	      destroy_fix_seg(root, parent);
	    } else {
	      if ((!wseg->left) || (!(wseg->left->mmin & SEG_COLOR_RED))) {
		/* CASE 3 */
		wseg->right->mmin &= ~SEG_COLOR_RED;
		wseg->mmin |= SEG_COLOR_RED;
		rotate_left_seg(root, wseg);
		wseg = parent->left;
	      }
	      /* CASE 4 */
	      wseg->mmin = (parent->mmin & SEG_COLOR_RED) |
		           (wseg->mmin & ~SEG_COLOR_RED);
	      parent->mmin &= ~SEG_COLOR_RED;
	      if (wseg->left) {
		wseg->left->mmin &= ~SEG_COLOR_RED;
	      }
	      rotate_right_seg(root, parent);
	    }
	  }
	} else {
	  /* Deleted node was red */
	  if (parent->left == yseg) {
	    parent->left = NULL;
	    parent->mmin = parent->maddr;
	  } else {
	    parent->right = NULL;
	    parent->mmax = parent->mend;
	  }
	}
      } else {
	*root = NULL;
      }
    }
    free_seg(yseg);
  }
  /* Color root black */
  if (*root) {
    (*root)->mmin &= ~SEG_COLOR_RED;
  }
}

static void destroy_fix_seg(struct seg_info **root, struct seg_info *seg)
{
  struct seg_info *wseg;

  while ((seg->parent) && (!(seg->mmin & SEG_COLOR_RED))) {
    if (seg == seg->parent->left) {
      wseg = seg->parent->right;
      if (wseg->mmin & SEG_COLOR_RED) {
	/* CASE 1 */
	wseg->mmin &= ~SEG_COLOR_RED;
	seg->parent->mmin |= SEG_COLOR_RED;
	rotate_left_seg(root, seg->parent);
	wseg = seg->parent->right;
      }
      if (((!wseg->left) || (!(wseg->left->mmin & SEG_COLOR_RED))) &&
	  ((!wseg->right) || (!(wseg->right->mmin & SEG_COLOR_RED)))) {
	/* CASE 2 */
	wseg->mmin |= SEG_COLOR_RED;
	seg = seg->parent;
      } else {
	if ((!wseg->right) || (!(wseg->right->mmin & SEG_COLOR_RED))) {
	  /* CASE 3 */
	  wseg->left->mmin &= ~SEG_COLOR_RED;
	  wseg->mmin |= SEG_COLOR_RED;
	  rotate_right_seg(root, wseg);
	  wseg = seg->parent->right;
	}
	/* CASE 4 */
	wseg->mmin = (seg->parent->mmin & SEG_COLOR_RED) |
	             (wseg->mmin & ~SEG_COLOR_RED);
	seg->parent->mmin &= ~SEG_COLOR_RED;
	wseg->right->mmin &= ~SEG_COLOR_RED;
	rotate_left_seg(root, seg->parent);
	break;
      }
    } else {
      wseg = seg->parent->left;
      if (wseg->mmin & SEG_COLOR_RED) {
	/* CASE 1 */
	wseg->mmin &= ~SEG_COLOR_RED;
	seg->parent->mmin |= SEG_COLOR_RED;
	rotate_right_seg(root, seg->parent);
	wseg = seg->parent->left;
      }
      if (((!wseg->left) || (!(wseg->left->mmin & SEG_COLOR_RED))) &&
	  ((!wseg->right) || (!(wseg->right->mmin & SEG_COLOR_RED)))) {
	/* CASE 2 */
	wseg->mmin |= SEG_COLOR_RED;
	seg = seg->parent;
      } else {
	if ((!wseg->left) || (!(wseg->left->mmin & SEG_COLOR_RED))) {
	  /* CASE 3 */
	  wseg->right->mmin &= ~SEG_COLOR_RED;
	  wseg->mmin |= SEG_COLOR_RED;
	  rotate_left_seg(root, wseg);
	  wseg = seg->parent->left;
	}
	/* CASE 4 */
	wseg->mmin = (seg->parent->mmin & SEG_COLOR_RED) |
	             (wseg->mmin & ~SEG_COLOR_RED);
	seg->parent->mmin &= ~SEG_COLOR_RED;
	wseg->left->mmin &= ~SEG_COLOR_RED;
	rotate_right_seg(root, seg->parent);
	break;
      }
    }
  }
  seg->mmin &= ~SEG_COLOR_RED;
}

static inline void print_space(FILE *fp, int n)
{
  while (n--) {
    putc(' ', fp);
  }
}

static void dump_seg_tree1(struct seg_info *root, int type, ULONG depth)
{
  if (root) {
    struct code_info *ci;

    if (root->right && (root->right->parent != root)) {
      fprintf(stdout, "Bad parent pointer!\n");
    }
    dump_seg_tree1(root->right, 1, depth + 4);
    print_space(stdout, depth);
    switch (type) {
    case 0: putc('-', stdout);
      break;
    case 1: putc('/', stdout);
      break;
    case 2: putc('\\', stdout);
      break;
    }
    fprintf(stdout, "%c:%lx:%lx:%lx:%lx", (root->mmin & SEG_COLOR_RED)?'O':'*',
	    root->mmin & ~SEG_COLOR_RED, root->maddr, root->mend, root->mmax);
    for (ci = root->codeinfo; ci; ci=ci->next) {
      fprintf(stdout, "->%08lx", ci->maddr);
    }
    fprintf(stdout, "\n");
    if (root->left && (root->left->parent != root)) {
      fprintf(stdout, "Bad parent pointer!\n");
    }
    dump_seg_tree1(root->left, 2, depth + 4);
  } else {
    print_space(stdout, depth);
    switch (type) {
    case 0: fprintf(stdout, "-[]\n");
      break;
    case 1: fprintf(stdout, "/[]\n");
      break;
    case 2: fprintf(stdout, "\\[]\n");
      break;
    }
  }
}

void dump_seg_tree(struct seg_info *root)
{
  dump_seg_tree1(root, 0, 0);
}


static struct seg_info *new_seg(ULONG maddr, ULONG mend)
{
  struct seg_info *si;

  if ((si = calloc(sizeof(*si),1))) {
    /* All pointers are NULL */
    /* New nodes are red */
    si->mmin = maddr | SEG_COLOR_RED;
    si->maddr = maddr;
    si->mend = mend;
    si->mmax = mend;
  } else {
    /* FAIL */
  }
  return (si);
}

static void free_seg(struct seg_info *seg)
{
  if (seg) {
    free(seg);
  }
}

/*
 * codeinfo handling
 */

static void free_codeinfo(struct code_info *ci)
{
  if (ci) {
    free(ci);
  }
}

struct code_info *find_ci(struct code_info **head, ULONG maddr)
{
  if (head) {
    struct code_info **node = head;

    while ((*node) && ((*node)->maddr != maddr)) {
      node = &((*node)->next);
    }
    
    if (*node) {
      struct code_info *ci = *node;

      if (head != node) {
	
	/* Unlink */
	*node = (*node)->next;
	/* Relink in at head */
	ci->next = (*head);
	*head = ci;
      }
      return(ci);
    }
  }
  return(NULL);
}

void clobber_code(ULONG maddr, ULONG val)
{
  struct seg_info *node;
  
#ifdef DEBUG
  printf("clobber_code(0x%08lx, 0x%08lx)\n", maddr, val);
#endif /* DEBUG */

  if ((node = find_seg(code_tree, maddr))) {
    struct code_info **ci = &(node->codeinfo);

    while (*ci) {
      if ((*ci)->maddr <= maddr) {
	struct code_info *deadci = *ci;
	/* Unlink */
	*ci = deadci->next;
	free_codeinfo(deadci);
      } else {
	ci = &((*ci)->next);
      }
    }
    if (!node->codeinfo) {
      /* Need to delete this node */
      
      destroy_seg(&code_tree, node); 
    }
  }
}
