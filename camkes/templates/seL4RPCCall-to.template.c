/*#
 *# Copyright 2014, NICTA
 *#
 *# This software may be distributed and modified according to the terms of
 *# the BSD 2-Clause license. Note that NO WARRANTY is provided.
 *# See "LICENSE_BSD2.txt" for details.
 *#
 *# @TAG(NICTA_BSD)
 #*/

#define _POSIX_SOURCE /* stpcpy */
#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <camkes/marshal.h>
#include <sel4/sel4.h>
#include <camkes/dataport.h>
#include <utils/util.h>

/*? macros.show_includes(me.to_instance.type.includes) ?*/
/*? macros.show_includes(me.to_interface.type.includes, '../static/components/' + me.to_instance.type.name + '/') ?*/

/*- set BUFFER_BASE = c_symbol('BUFFER_BASE') -*/
/*- set base = '((void*)&seL4_GetIPCBuffer()->msg[0])' -*/
/*- set userspace_ipc = False -*/
/*- if configuration -*/
    /*- set buffers = filter(lambda('x: x.instance == \'%s\' and x.attribute == \'%s_buffer\'' % (me.to_instance.name, me.to_interface.name)), configuration.settings) -*/
    /*- if len(buffers) == 1 -*/
        /*- set base = buffers[0].value -*/
        /*- set userspace_ipc = True -*/
        extern void * /*? base ?*/;
    /*- endif -*/
/*- endif -*/
#define /*? BUFFER_BASE ?*/ /*? base ?*/

/*- set methods_len = len(me.to_interface.type.methods) -*/

/*- for m in me.to_interface.type.methods -*/
    extern
    /*- if m.return_type -*/
        /*- if m.return_type.array -*/
            /*- if isinstance(m.return_type, camkes.ast.Type) and m.return_type.type == 'string' -*/
                char **
            /*- else -*/
                /*? show(m.return_type) ?*/ *
            /*- endif -*/
        /*- elif isinstance(m.return_type, camkes.ast.Type) and m.return_type.type == 'string' -*/
            char *
        /*- else -*/
            /*? show(m.return_type) ?*/
        /*- endif -*/
    /*- else -*/
        void
    /*- endif -*/
    /*? me.to_interface.name ?*/_/*? m.name ?*/(
        /*- if m.return_type and m.return_type.array -*/
            /*- set ret_sz = c_symbol('ret_sz') -*/
            size_t * /*? ret_sz ?*/
            /*- if len(m.parameters) > 0 -*/
                ,
            /*- endif -*/
        /*- endif -*/
        /*? ', '.join(map(show, m.parameters)) ?*/
    );

/*- set name = m.name -*/
/*- set function = '%s_unmarshal' % m.name -*/
/*- set buffer = base -*/
/*- set sizes = [None] -*/
/*- if userspace_ipc -*/
    /*- do sizes.__setitem__(0, 'PAGE_SIZE_4K') -*/
/*- else -*/
    /*- do sizes.__setitem__(0, 'seL4_MsgMaxLength * sizeof(seL4_Word)') -*/
/*- endif -*/
/*- set size = sizes[0] -*/
/*- set input_parameters = filter(lambda('x: x.direction.direction in [\'in\', \'inout\']'), m.parameters) -*/
/*- include 'unmarshal-inputs.c' -*/

/*- set function = '%s_marshal' % m.name -*/
/*- set output_parameters = filter(lambda('x: x.direction.direction in [\'out\', \'inout\']'), m.parameters) -*/
/*- set return_type = m.return_type -*/
/*- include 'marshal-outputs.c' -*/

/*- endfor -*/

/*- set ep = alloc('ep', seL4_EndpointObject, read=True, write=True) -*/

static seL4_Word /*? me.to_interface.name ?*/_badge = 0;

seL4_Word /*? me.to_interface.name ?*/_get_badge(void) {
    return /*? me.to_interface.name ?*/_badge;
}

int /*? me.to_interface.name ?*/__run(void) {
    /*- set info = c_symbol('info') -*/
    seL4_MessageInfo_t /*? info ?*/ = seL4_Wait(/*? ep ?*/, & /*? me.to_interface.name ?*/_badge);
    while (1) {
        /*- if not options.fcall_leave_reply_cap or len(me.to_instance.type.provides + me.to_instance.type.uses + me.to_instance.type.consumes + me.to_instance.type.mutexes + me.to_instance.type.semaphores) > 1 -*/
            /* We need to save the reply cap because the user's implementation may
             * perform operations that overwrite or discard it.
             */
            /*- set result = c_symbol() -*/
            /*- set cnode = alloc_cap('cnode', my_cnode, write=True) -*/
            /*- set reply_cap_slot = alloc_cap('reply_cap_slot', None) -*/
            int /*? result ?*/ UNUSED = seL4_CNode_SaveCaller(/*? cnode ?*/, /*? reply_cap_slot ?*/, 32);
            assert(/*? result ?*/ == 0);
        /*- endif -*/

        /*- set call = c_symbol('call') -*/
        /*- if methods_len <= 1 -*/
            unsigned int /*? call ?*/ = 0;
        /*- else -*/
            /*- if methods_len <= 2 ** 8 -*/
                uint8_t
            /*- elif methods_len <= 2 ** 16 -*/
                uint16_t
            /*- elif methods_len <= 2 ** 32 -*/
                uint32_t
            /*- elif methods_len <= 2 ** 64 -*/
                uint64_t
            /*- else -*/
                /*? raise(Exception('too many methods in interface %s' % me.to_interface.type.name)) ?*/
            /*- endif -*/
            /*? call ?*/;

            /*- set buffer = c_symbol('buffer') -*/
            void * /*? buffer ?*/ = (void*)/*? BUFFER_BASE ?*/;

            memcpy(& /*? call ?*/, /*? buffer ?*/, sizeof(/*? call ?*/));
        /*- endif -*/

        switch (/*? call ?*/) {
            /*- for i, m in enumerate(me.to_interface.type.methods) -*/
                case /*? i ?*/: { /*? '/' + '* ' + m.name + ' *' + '/' ?*/
                    /*# Declare parameters. #*/
                    /*- for p in m.parameters -*/

                        /*- if p.array -*/
                            size_t /*? p.name ?*/_sz;
                            /*- if isinstance(p.type, camkes.ast.Type) and p.type.type == 'string' -*/
                                char ** /*? p.name ?*/ = NULL;
                            /*- else -*/
                                /*? show(p.type) ?*/ * /*? p.name ?*/ = NULL;
                            /*- endif -*/
                        /*- elif isinstance(p.type, camkes.ast.Type) and p.type.type == 'string' -*/
                            char * /*? p.name ?*/ = NULL;
                        /*- else -*/
                            /*? show(p.type) ?*/ /*? p.name ?*/;
                        /*- endif -*/
                    /*- endfor -*/

                    /* Unmarshal parameters */
                    /*- set function = '%s_unmarshal' % m.name -*/
                    /*- set input_parameters = filter(lambda('x: x.direction.direction in [\'in\', \'inout\']'), m.parameters) -*/
                    /*- include 'call-unmarshal-inputs.c' -*/;

                    /* Call the implementation */
                    /*- set ret = c_symbol('ret') -*/
                    /*- set ret_sz = c_symbol('ret_sz') -*/
                    /*- if m.return_type -*/
                        /*- if m.return_type.array -*/
                            size_t /*? ret_sz ?*/;
                            /*- if isinstance(m.return_type, camkes.ast.Type) and m.return_type.type == 'string' -*/
                                char **
                            /*- else -*/
                                /*? show(m.return_type) ?*/ *
                            /*- endif -*/
                        /*- elif isinstance(m.return_type, camkes.ast.Type) and m.return_type.type == 'string' -*/
                            char *
                        /*- else -*/
                            /*? show(m.return_type) ?*/
                        /*- endif -*/
                        /*? ret ?*/ =
                    /*- endif -*/
                    /*? me.to_interface.name ?*/_/*? m.name ?*/(
                        /*- if m.return_type and m.return_type.array -*/
                            & /*? ret_sz ?*/
                            /*- if len(m.parameters) > 0 -*/
                                ,
                            /*- endif -*/
                        /*- endif -*/
                        /*- for p in m.parameters -*/
                            /*- if p.array -*/
                                /*- if p.direction.direction in ['inout', 'out'] -*/
                                    &
                                /*- endif -*/
                                /*? p.name ?*/_sz,
                            /*- endif -*/
                            /*- if p.direction.direction in ['inout', 'out'] -*/
                                &
                            /*- endif -*/
                            /*? p.name ?*/
                            /*- if not loop.last -*/,/*- endif -*/
                        /*- endfor -*/
                    );

                    /* Marshal the response */
                    /*- set function = '%s_marshal' % m.name -*/
                    /*- set output_parameters = filter(lambda('x: x.direction.direction in [\'out\', \'inout\']'), m.parameters) -*/
                    /*- set return_type = m.return_type -*/
                    /*- set length = c_symbol('length') -*/
                    unsigned int /*? length ?*/ = /*- include 'call-marshal-outputs.c' -*/;

                    /*# We no longer need anything we previously malloced #*/
                    /*- if m.return_type and (m.return_type.array or (isinstance(m.return_type, camkes.ast.Type) and m.return_type.type == 'string')) -*/
                        free(/*? ret ?*/);
                    /*- endif -*/
                    /*- for p in m.parameters -*/
                        /*- if p.array or (isinstance(p.type, camkes.ast.Type) and p.type.type == 'string') -*/
                            free(/*? p.name ?*/);
                        /*- endif -*/
                    /*- endfor -*/

                    /*? info ?*/ = seL4_MessageInfo_new(0, 0, 0, /* length */
                        /*- if userspace_ipc -*/
                            0
                        /*- else -*/
                            ROUND_UP(/*? length ?*/, sizeof(seL4_Word)) / sizeof(seL4_Word)
                        /*- endif -*/
                    );

                    /* Send the response */
                    /*- if not options.fcall_leave_reply_cap or len(me.to_instance.type.provides + me.to_instance.type.uses + me.to_instance.type.consumes + me.to_instance.type.mutexes + me.to_instance.type.semaphores) > 1 -*/
                        seL4_Send(/*? reply_cap_slot ?*/, /*? info ?*/);
                        /*? info ?*/ = seL4_Wait(/*? ep ?*/, & /*? me.to_interface.name ?*/_badge);
                    /*- else -*/

                        /*# The optimisation below is only valid to perform if we do not have any
                         *# reference (typedefed C) types.
                         #*/
                        /*- set contains_reference_type = [False] -*/
                        /*- for p in m.parameters -*/
                          /*- if isinstance(p.type, camkes.ast.Reference) -*/
                            /*- do contains_reference_type.__setitem__(0, True) -*/
                            /*- break -*/
                          /*- endif -*/
                        /*- endfor -*/

                        /*- if options.fspecialise_syscall_stubs and not contains_reference_type[0] and len(filter(lambda('x: x.array or x.type.type == \'string\''), m.parameters)) == 0 -*/
#ifdef ARCH_ARM
#ifndef __SWINUM
    #define __SWINUM(x) ((x) & 0x00ffffff)
#endif
                            /*- if methods_len == 1 and not m.return_type and len(m.parameters) == 0 -*/
                                /* We don't need to send or receive any information, so
                                 * we can call ReplyWait with a custom syscall stub
                                 * that reduces the overhead of the call. To explain
                                 * where this deviates from the standard ReplyWait
                                 * stub:
                                 *  - No asm clobbers because we're not receiving any
                                 *    arguments in the message;
                                 *  - The MessageInfo as an input only because we know
                                 *    the return (a new Call) will be 0 as well; and
                                 *  - Setup r7 and r1 first because they are preserved
                                 *    across the syscall and this helps the compiler
                                 *    make a tighter loop if necessary.
                                 */
                                /*- set scno = c_symbol() -*/
                                register seL4_Word /*? scno ?*/ asm("r7") = seL4_SysReplyWait;
                                /*- set info2 = c_symbol() -*/
                                register seL4_MessageInfo_t /*? info2 ?*/ asm("r1") = seL4_MessageInfo_new(0, 0, 0, 0);
                                /*- set src = c_symbol() -*/
                                register seL4_Word /*? src ?*/ asm("r0") = /*? ep ?*/;
                                asm volatile("swi %[swinum]"
                                    :"+r"(/*? src ?*/)
                                    :[swinum]"i"(__SWINUM(seL4_SysReplyWait)), "r"(/*? scno ?*/), "r"(/*? info2 ?*/)
                                );
                                /*? info ?*/ = /*? info2 ?*/; /*# Most probably, not necessary. #*/
                                break;
                            /*- endif -*/
#endif
                        /*- endif -*/
                        /*? info ?*/ = seL4_ReplyWait(/*? ep ?*/, /*? info ?*/, & /*? me.to_interface.name ?*/_badge);
                    /*- endif -*/

                    break;
                }
            /*- endfor -*/
            default: {
                /*# TODO: Handle error #*/
                assert(0);
            }
        }
    }

    UNREACHABLE();
}
