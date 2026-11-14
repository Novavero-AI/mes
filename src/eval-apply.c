/* -*-comment-start: "//";comment-end:""-*-
 * GNU Mes --- Maxwell Equations of Software
 * Copyright © 2016,2017,2018,2019,2020,2022,2025 Janneke Nieuwenhuizen <janneke@gnu.org>
 * Copyright © Timothy Sample 2022,2026 <samplet@ngyro.com>
 * Copyright © 2025,2026 Ekaitz Zarraga <ekaitz@elenq.tech>
 *
 * This file is part of GNU Mes.
 *
 * GNU Mes is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 *
 * GNU Mes is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Mes.  If not, see <http://www.gnu.org/licenses/>.
 */

/** Commentary:
    Evaluation machinery.
 */

/** Code: */

#include "mes/lib.h"
#include "mes/mes.h"

#include <string.h>

#if __M2__
void
stack_init ()
{
  g_stack = 0;
}
struct scm *
stack_index (long i)
{
  char *c = (char *)STACK + i * M2_PTR_SIZE;
  struct scm **t = (struct scm **)c;
  return *t;
}
void
stack_set (long i, struct scm *v)
{
  char *c = (char *)(STACK) + i * M2_PTR_SIZE;
  struct scm **t = (struct scm **)c;
  *t = v;
}
void
stack_push (struct scm *x)
{
  stack_set (g_stack, x);
  g_stack = g_stack + 1;
}
struct scm *
stack_pop ()
{
  g_stack = g_stack - 1;
  return stack_index (g_stack);
}
void
stack_drop (long n)
{
  g_stack -= n;
}
struct scm *
stack_peek ()
{
  return stack_index (g_stack - 1);
}
int
stack_empty_p ()
{
  return (g_stack == 0);
}
void
stack_rewind_to (long to)
{
  stack_drop (g_stack - to);
}
#endif /* __M2__*/

void
stack_print ()    /* for debugging */
{
  int i;
  for (i = g_stack - 1; i >= 0; i--)
    {
      eputs (ltoa (i));
      eputs (":\t");
      display_ (stack_index (i));
      eputs ("\n");
    }
}

struct scm *
stack_to_scm ()
{
  size_t i;
  struct scm *stack_vector = make_vector_uninit_ (g_stack);
  for (i = 0; i < g_stack; i++)
    {
      struct scm *val = stack_index (i);
      vector_set_x_ (stack_vector, i, val);
    }
  return stack_vector;
}

void
stack_from_scm (struct scm *stack)
{
  size_t l = vector_length (stack)->value;
  for (g_stack = 0; g_stack < l; 0)
    {
      struct scm *x = vector_ref_ (stack, g_stack);
      stack_push (x);
    }
}

struct scm *
make_continuation (struct scm *proc, unsigned short fp, unsigned short pc)
{
  return make_cell (TCONTINUATION, make_frame (proc, pc, fp), stack_to_scm ());
}

struct scm *
apply (struct scm *fn, struct scm *x)
{
  /* No need to eval, just call the function */
  if (fn->type == TBUILTIN)
    {
      switch (builtin_arity_ (fn))
        {
        case 0:
          {
            long argno = length__ (x);
            if (0 != argno)
              error (cell_symbol_wrong_number_of_args,
                     cons (make_number (argno), builtin_name (fn)));
            function0_t fp = (function0_t) builtin_function (fn);
            return fp ();
          }
        case 1:
          {
            long argno = length__ (x);
            if (1 != argno)
              error (cell_symbol_wrong_number_of_args,
                     cons (make_number (argno), builtin_name (fn)));
            function1_t fp = (function1_t) builtin_function (fn);
            return fp (x->car);
          }
        case 2:
          {
            long argno = length__ (x);
            if (2 != argno)
              error (cell_symbol_wrong_number_of_args,
                     cons (make_number (argno), builtin_name (fn)));
            function2_t fp = (function2_t) builtin_function (fn);
            return fp (x->car, x->cdr->car);
          }
        case 3:
          {
            long argno = length__ (x);
            if (3 != argno)
              error (cell_symbol_wrong_number_of_args,
                     cons (make_number (argno), builtin_name (fn)));
            function3_t fp = (function3_t) builtin_function (fn);
            return fp (x->car, x->cdr->car, x->cdr->cdr->car);
          }
        case -1:
          {
            function1_t fp = (function1_t) builtin_function (fn);
            return fp (x);
          }
        default:
          assert_msg (0, "apply: Unreachable arity in builtin.");
          return cell_nil;
        }
    }

  /* We need to eval code: we create an empty frame as a barrier so we now when
   * to pause evaluation to come back here.
   */
  struct scm *barrier = make_frame (cell_nil, 0, 0);
  stack_push (barrier);     /* `apply` barrier */
  stack_push (x);
  stack_push (fn);
  struct scm *res = eval_compiled ();   /* returns to the barrier */
  stack_pop ();
  return res;
}

/* The low-level evaluator
 */
struct scm *
display_backtrace (struct scm *kont)
{
  if (kont->type != TCONTINUATION)
    error (cell_symbol_wrong_type_arg,
           cons (kont, cstring_to_symbol ("display-backtrace")));
  struct scm *proc_name;
  struct scm *locals;
  struct scm *frame = kont->state;
  struct scm *stack = kont->stack;
  while (frame->closure != cell_nil)
    {
      unsigned int lineno = frame->frame.pc;
      if (frame->closure != cell_nil)
        {
          proc_name = template_name (frame->closure->template);
          locals = template_locals (frame->closure->template);
        }
      display_error_ (proc_name);
      eputs (":");
      eputs (ltoa (lineno));
      eputs ("\n");
      unsigned int i = 0;
      unsigned int l = locals->length;
      for (i = 0; i < l; i = i + 1)
        {
          struct scm *binding = vector_ref_ (stack, frame->frame.fp + 1 + i);
          eputs (" - ");
          display_error_ (vector_ref_ (locals, i));
          eputs (" = ");
          display_error_ (binding->name);
          eputs (" -> ");
          display_error_ (binding->binding);
          eputs ("\n");
        }
      frame = vector_ref_ (stack, frame->frame.fp);
    }
  return cell_unspecified;
}

struct scm *
eval_compiled ()
{
  /* NOTE: We use the STACK for everything in this function */

  unsigned char inst = OP_UNREACHABLE;
  unsigned int pc = 0;
  unsigned int fp = g_stack - 3;

  /* These live in Scheme memory */
  struct scm *mark = cell_nil;
  struct scm *proc = cell_nil;
  unsigned char *code;
  struct scm *literals = cell_nil;      /* This is cached */

  /* Internal temporary values that are shared in fallthroughs */
  struct scm *fn = cell_nil;
  long argno = 0;

#if (__riscv && (__GNUC__ || __TINYC__)) || (!__riscv && !__M2__)
  int res = setjmp (JMP_BUF);
  if (res != 0)
    goto throw;
#endif

  /* Get this thing rolling! */
  goto apply;

  while (1)
    {
      inst = code [pc];
      pc += 1;

      switch (inst)
        {
        case OP_GC:
          stack_push (mark);      /* Preserve from garbage collection */
          stack_push (proc);      /* Preserve from garbage collection */
          gc ();
          proc = stack_pop ();
          mark = stack_pop ();
          code = cell_bytes (template_code (proc->template));
          literals = cell_nil;    /* Invalidate cache */
          stack_push (cell_unspecified);
          break;
        case OP_REF_MARK:
          stack_push (mark);
          break;
        case OP_SET_MARK:
          mark = stack_pop ();
          stack_push (cell_unspecified);
          break;
        case OP_UNSPECIFIED:
          stack_push (cell_unspecified);
          break;
        case OP_NIL:
          stack_push (cell_nil);
          break;
        case OP_FALSE:
          stack_push (cell_f);
          break;
        case OP_TRUE:
          stack_push (cell_t);
          break;
        case OP_LITERAL:
          {
            pc += pc & 1;       /* Align to half size */
            unsigned short *t = (unsigned short *)(code + pc);
            unsigned short literal_id = *t;
            pc += 2;
            if (literals == cell_nil)   /* If not set, cache it for later */
              literals = template_literals (proc->template);
            stack_push (vector_ref_ (literals, literal_id));
            break;
          }
        case OP_DROP:
          stack_pop ();
          break;

        case OP_BRANCH:
          {
            pc += pc & 1;       /* Align to half size */
            unsigned short *t = (unsigned short *)(code + pc);
            unsigned short target = *t;
            pc += 2;
            if (stack_pop () == cell_f)
              pc = target;
            break;
          }
        case OP_JUMP:
          {
            pc += pc & 1;       /* Align to half size */
            unsigned short *t = (unsigned short *)(code + pc);
            unsigned short target = *t;
            pc += 2;
            pc = target;
            break;
          }

        case OP_REF_LOCAL:
          {
            pc += pc & 1;       /* Align to half size */
            unsigned short *t = (unsigned short *)(code + pc);
            unsigned short target = *t;
            pc += 2;
            stack_push (stack_index (fp + 1 + target));
            break;
          }
        case OP_REF_CLOSURE:
          {
            pc += pc & 1;       /* Align to half size */
            unsigned short *t = (unsigned short *)(code + pc);
            unsigned short target = *t;
            pc += 2;
            stack_push (vector_ref_ (proc->env, target));
            break;
          }
        case OP_RESOLVE:
          {
            struct scm *target = stack_pop ();

            if (target->type != TBINDING)
              assert_msg (0, "eval_compiled: OP_RESOLVE: expected binding.");
            if (target->binding->type != TVARIABLE)
              assert_msg (0, "eval_compiled: expected variable in binding");

            stack_push (target->binding->variable);

            if (stack_peek () == cell_undefined)
              error (cell_symbol_unbound_variable, cons (target->name, proc));
            break;
          }
        case OP_SET:
          {
            struct scm *target = stack_pop ();

            if (target->type != TBINDING)
              assert_msg (0, "eval_compiled: OP_SET: expected binding");
            if (target->binding->type != TVARIABLE)
              assert_msg (0, "eval_compiled: expected variable in binding");

            target->binding->variable = stack_pop ();
            stack_push (cell_unspecified);
            break;
          }
          break;

        case OP_CLOSURE:
          {
            pc += pc & 1;       /* Align to half size */
            unsigned short *t = (unsigned short *)(code + pc);
            argno = *t;
            pc += 2;
            struct scm *template;
            struct scm *captures = make_vector_ (argno, cell_nil);
            for (; argno > 0; argno -= 1)
              vector_set_x_ (captures, argno - 1, stack_pop ());
            template = stack_pop ();
            stack_push (make_closure (captures, template));
            break;
          }

        throw:
          {
            struct scm *throw =
              module_variable (initial_module (), cell_symbol_throw);
            if (throw == cell_f)    /* Throw not defined yet, just die */
              {
                display_error_ (stack_pop ());
                eputs ("\n");
                exit (1);
              }
            else
              stack_push (throw->variable);
            /* fallthrough */
          }

        case OP_APPLY:
        case OP_TAIL_APPLY:
          {
          apply:
            fn = stack_pop ();
            if (fn->type == TBUILTIN)
              if (builtin_arity_ (fn) == -1)
                /* Optimization: the args are ready in the stack in a list,
                 * don't unpack to pack them later
                 */
                goto builtin_1;
            struct scm *args = stack_pop ();
            struct scm *cur = cell_nil;
            argno = 0;
            for (cur = args; cur != cell_nil; cur = cur->cdr)
              {
                stack_push (cur->car);
                argno++;
              }

            if (inst == OP_TAIL_APPLY)
              inst = OP_TAIL_CALL;
            goto call;
          }
        case OP_CALL:
        case OP_TAIL_CALL:
          {
            pc += pc & 1;       /* Align to half size */
            unsigned short *t = (unsigned short *)(code + pc);
            argno = *t;
            pc += 2;
            fn = stack_pop ();
          call:
            switch (fn->type)
              {
              case TCLOSURE:
                {
                  int arity = template_arity (fn->template)->value;

                  if ((arity >= 0 && argno != arity) ||
                      (arity < 0 && (argno + arity < -1)))
                    error (cell_symbol_wrong_number_of_args,
                           cons (make_number (argno),
                                 template_name (fn->template)));

                  if (arity < 0)
                    {
                      /* Extract the _rest_:
                       * They are reversed in the stack, so we cons them in
                       * the correct order and push them back to the stack.
                       */
                      arity = -arity;
                      struct scm *rest_args = cell_nil;

                      for (; argno >= arity; argno--)
                        rest_args = cons (stack_pop (), rest_args);
                      stack_push (rest_args);
                    }

                  /* At this point the stack looks like this:
                   *
                   * FRAME_i ... ARG_0 ... ARG_n
                   *             ^-------------^ ^
                   *             Args            g_stack
                   */
                  size_t args_loc = g_stack - arity;
                  size_t frame_loc = g_stack;
                  stack_push (cell_unspecified);  /* Make space for the frame header */

                  /* Add locals to environment */
                  struct scm *l = template_locals (fn->template);
                  long j, local_count = l->length;
                  for (j = 0; j < local_count; j++)
                    {
                      struct scm *name = vector_ref_ (l, j);
                      struct scm *target = cell_undefined;
                      if (j < arity)
                        target = stack_index (args_loc + j);
                      stack_push (make_binding (name, make_variable (target)));
                    }

                  /* At this point the stack looks like this:
                   *
                   * FRAME_i ... ARG_0 ... ARG_n *unspecified* BIND_0 ... BIND_n ... BIND_m
                   * ^           ^-------------^               ^---------------^^---------^ ^
                   *             Args                          Locals: (args)    (rest)     g_stack
                   */

                  /* Put the frame header */
                  struct scm *frame_header = stack_index (fp);   /* TCO: Reuse the frame */
                  size_t current_fp = fp;
                  if (inst != OP_TAIL_CALL)
                    {
                      struct scm *to = proc;
                      if (mark != cell_nil)
                        to = cons (mark, proc);
                      frame_header = make_frame (to, pc, fp);
                      current_fp = args_loc;
                      mark = cell_nil;
                    }
                  stack_set (frame_loc, frame_header);

                  /* At this point the stack looks like this:
                   *
                   * FRAME_i ... ARG_0 ... ARG_n FRAME_i+1 BIND_0 ... BIND_n ... BIND_m
                   * ^           ^-------------^           ^---------------^^---------^ ^
                   *             Args                      Locals: (args)    (rest)     g_stack
                   * In the TCO case:
                   *
                   * FRAME_i ... ARG_0 ... ARG_n FRAME_i BIND_0 ... BIND_n ... BIND_m
                   * ^           ^-------------^ ^       ^---------------^^---------^ ^
                   * |           Args            |       Locals: (args)    (rest)     g_stack
                   * +-- Same -------------------+
                   */

                  size_t len = g_stack - frame_loc;
                  for (j = 0; j < len; j++)
                    stack_set (current_fp + j, stack_index (frame_loc + j));
                  g_stack -= frame_loc - current_fp;

                  /* At this point the stack looks like this:
                   *
                   * FRAME_i ... FRAME_i+1 BIND_0 ... BIND_n ... BIND_m
                   * ^           ^         ^---------------^^---------^ ^
                   *             fp        Locals: (args)    (rest)     g_stack
                   *
                   * In the TCO case:
                   *
                   * FRAME_i BIND_0 ... BIND_n ... BIND_m
                   * ^       ^---------------^^---------^ ^
                   * fp      Locals: (args)    (rest)     g_stack
                   */

                  /* Try to garbage collect.
                   * This is the only place we run the GC.
                   */
                  stack_push (mark);     /* Preserve from garbage collection */
                  stack_push (fn);       /* Preserve from garbage collection */
                  gc_check ();
                  fn = stack_pop ();
                  mark = stack_pop ();

                  proc = fn;
                  fp = current_fp;
                  pc = 0;
                  code = cell_bytes (template_code (proc->template));
                  literals = cell_nil;    /* Invalidate cache */
                  break;
                }

              case TBUILTIN:
                switch (builtin_arity_ (fn))
                  {
                  case 0:
                    {
                      if (0 != argno)
                        error (cell_symbol_wrong_number_of_args,
                               cons (make_number (argno), builtin_name (fn)));
                      function0_t fp = (function0_t) builtin_function (fn);
                      stack_push (fp ());
                      break;
                    }
                  case 1:
                    {
                      struct scm *a1;
                      if (1 != argno)
                        error (cell_symbol_wrong_number_of_args,
                               cons (make_number (argno), builtin_name (fn)));
builtin_1:
                      a1 = stack_pop ();
                      function1_t fp = (function1_t) builtin_function (fn);
                      stack_push (fp (a1));
                      break;
                    }
                  case 2:
                    {
                      struct scm *a2, *b2;
                      if (2 != argno)
                        error (cell_symbol_wrong_number_of_args,
                               cons (make_number (argno), builtin_name (fn)));
                      function2_t fp = (function2_t) builtin_function (fn);
                      b2 = stack_pop ();
                      a2 = stack_pop ();
                      stack_push (fp (a2, b2));
                      break;
                    }
                  case 3:
                    {
                      struct scm *a3, *b3, *c3;
                      if (3 != argno)
                        error (cell_symbol_wrong_number_of_args,
                               cons (make_number (argno), builtin_name (fn)));
                      function3_t fp = (function3_t) builtin_function (fn);
                      c3 = stack_pop ();
                      b3 = stack_pop ();
                      a3 = stack_pop ();
                      stack_push (fp (a3, b3, c3));
                      break;
                    }
                  case -1:
                    {
                      /* Arguments are in the stack one by one */
                      struct scm *cur;
                      for (cur = cell_nil; argno != 0; argno--)
                        cur = cons (stack_pop (), cur);
                      stack_push (cur);
                      goto builtin_1;
                    }
                  default:
                    assert_msg (0, "builtin: unreachable arity");
                    break;
                  }
                break;

              case TCONTINUATION:
                {
                  /* Only uses the first argument, discards the rest */
                  struct scm *arg = stack_pop ();

                  stack_from_scm (fn->stack);

                  proc = fn->state->closure;
                  if (proc->type == TPAIR)
                    {
                      mark = proc->car;
                      proc = proc->cdr;
                    }
                  fp = fn->state->frame.fp;
                  pc = fn->state->frame.pc;
                  code = cell_bytes (template_code (proc->template));
                  literals = cell_nil;    /* Invalidate cache */

                  stack_push (arg);
                  break;
                }
              default:
                assert_msg (0, "OP_[TAIL_]CALL: Wrong type to apply.");
                break;
              }
            break;
          }
        case OP_RETURN:
          {
            /* At this point the stack looks like this:
             *
             * FRAME_i ... FRAME_i+1 BIND_0 ... BIND_n ... BIND_m RESULT
             * ^           ^         ^---------------^^---------^        ^
             *             fp        Locals: (args)    (rest)            g_stack
             */
            struct scm *res   = stack_peek ();
            struct scm *frame = stack_index (fp);
            stack_rewind_to (fp);

            struct scm *fn = frame->closure;

            /* We have marks */
            if (fn->type == TPAIR)
              {
                mark = fn->car;
                fn = fn->cdr;
              }
            else
              mark = cell_nil;

            /* We came from a C `apply` */
            if (fn == cell_nil)
              return res;

            stack_push (res);

            proc = fn;
            fp = frame->frame.fp;
            pc = frame->frame.pc;
            code = cell_bytes (template_code (proc->template));
            literals = cell_nil;        /* Invalidate cache */
            break;
          }
        case OP_CC:
          {
            pc += pc & 1;       /* Align to half size */
            unsigned short *t = (unsigned short *)(code + pc);
            unsigned short target = *t;
            pc += 2;
            struct scm *to = proc;
            if (mark != cell_nil)
              to = cons (mark, proc);
            struct scm *cont = make_continuation (to, fp, target);
            stack_push (cont);
            break;
          }
        case OP_UNREACHABLE:
          assert_msg (0, "eval_compiled: unreachable code");
          break;
        default:
          assert_msg (0, "eval_compiled: unknown instruction");
          break;
        }
    }
}
