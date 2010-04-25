// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 2009 Simon Howard
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
// 02111-1307, USA.
//
// DESCRIPTION:
//     Queue of waiting callbacks, stored in a binary min heap, so that we
//     can always get the first callback.
//
//-----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "opl_queue.h"

#define MAX_OPL_QUEUE 64

typedef struct
{
    opl_callback_t callback;
    void *data;
    unsigned int time;
} opl_queue_entry_t;

struct opl_callback_queue_s
{
    opl_queue_entry_t entries[MAX_OPL_QUEUE];
    unsigned int num_entries;
};

opl_callback_queue_t *OPL_Queue_Create(void)
{
    opl_callback_queue_t *queue;

    queue = malloc(sizeof(opl_callback_queue_t));
    queue->num_entries = 0;

    return queue;
}

void OPL_Queue_Destroy(opl_callback_queue_t *queue)
{
    free(queue);
}

int OPL_Queue_IsEmpty(opl_callback_queue_t *queue)
{
    return queue->num_entries == 0;
}

void OPL_Queue_Clear(opl_callback_queue_t *queue)
{
    queue->num_entries = 0;
}

void OPL_Queue_Push(opl_callback_queue_t *queue,
                    opl_callback_t callback, void *data,
                    unsigned int time)
{
    int entry_id;
    int parent_id;

    if (queue->num_entries >= MAX_OPL_QUEUE)
    {
        fprintf(stderr, "OPL_Queue_Push: Exceeded maximum callbacks\n");
        return;
    }

    // Add to last queue entry.

    entry_id = queue->num_entries;
    ++queue->num_entries;

    // Shift existing entries down in the heap.

    while (entry_id > 0)
    {
        parent_id = (entry_id - 1) / 2;

        // Is the heap condition satisfied?

        if (time >= queue->entries[parent_id].time)
        {
            break;
        }

        // Move the existing entry down in the heap.

        memcpy(&queue->entries[entry_id],
               &queue->entries[parent_id],
               sizeof(opl_queue_entry_t));

        // Advance to the parent.

        entry_id = parent_id;
    }

    // Insert new callback data.

    queue->entries[entry_id].callback = callback;
    queue->entries[entry_id].data = data;
    queue->entries[entry_id].time = time;
}

int OPL_Queue_Pop(opl_callback_queue_t *queue,
                  opl_callback_t *callback, void **data)
{
    opl_queue_entry_t *entry;
    int child1, child2;
    int i, next_i;

    // Empty?

    if (queue->num_entries <= 0)
    {
        return 0;
    }

    // Store the result:

    *callback = queue->entries[0].callback;
    *data = queue->entries[0].data;

    // Decrease the heap size, and keep pointer to the last entry in
    // the heap, which must now be percolated down from the top.

    --queue->num_entries;
    entry = &queue->entries[queue->num_entries];

    // Percolate down.

    i = 0;

    for (;;)
    {
        child1 = i * 2 + 1;
        child2 = i * 2 + 2;

        if (child1 < queue->num_entries
         && queue->entries[child1].time < entry->time)
        {
            // Left child is less than entry.
            // Use the minimum of left and right children.

            if (child2 < queue->num_entries
             && queue->entries[child2].time < queue->entries[child1].time)
            {
                next_i = child2;
            }
            else
            {
                next_i = child1;
            }
        }
        else if (child2 < queue->num_entries
              && queue->entries[child2].time < entry->time)
        {
            // Right child is less than entry.  Go down the right side.

            next_i = child2;
        }
        else
        {
            // Finished percolating.
            break;
        }

        // Percolate the next value up and advance.

        memcpy(&queue->entries[i],
               &queue->entries[next_i],
               sizeof(opl_queue_entry_t));
        i = next_i;
    }

    // Store the old last-entry at its new position.

    memcpy(&queue->entries[i], entry, sizeof(opl_queue_entry_t));

    return 1;
}

unsigned int OPL_Queue_Peek(opl_callback_queue_t *queue)
{
    if (queue->num_entries > 0)
    {
        return queue->entries[0].time;
    }
    else
    {
        return 0;
    }
}

#ifdef TEST

#include <assert.h>

static void PrintQueueNode(opl_callback_queue_t *queue, int node, int depth)
{
    int i;

    if (node >= queue->num_entries)
    {
        return;
    }

    for (i=0; i<depth * 3; ++i)
    {
        printf(" ");
    }

    printf("%i\n", queue->entries[node].time);

    PrintQueueNode(queue, node * 2 + 1, depth + 1);
    PrintQueueNode(queue, node * 2 + 2, depth + 1);
}

static void PrintQueue(opl_callback_queue_t *queue)
{
    PrintQueueNode(queue, 0, 0);
}

int main()
{
    opl_callback_queue_t *queue;
    int iteration;

    queue = OPL_Queue_Create();

    for (iteration=0; iteration<5000; ++iteration)
    {
        opl_callback_t callback;
        void *data;
        unsigned int time;
        unsigned int newtime;
        int i;

        for (i=0; i<MAX_OPL_QUEUE; ++i)
        {
            time = rand() % 0x10000;
            OPL_Queue_Push(queue, NULL, NULL, time);
        }

        time = 0;

        for (i=0; i<MAX_OPL_QUEUE; ++i)
        {
            assert(!OPL_Queue_IsEmpty(queue));
            newtime = OPL_Queue_Peek(queue);
            assert(OPL_Queue_Pop(queue, &callback, &data));

            assert(newtime >= time);
            time = newtime;
        }

        assert(OPL_Queue_IsEmpty(queue));
        assert(!OPL_Queue_Pop(queue, &callback, &data));
    }
}

#endif

