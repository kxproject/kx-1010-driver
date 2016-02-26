/* 1010 PCI/PCIe Audio Driver
   Copyright (c) Eugene Gavrilov, 2002-2016

   * This program is free software; you can redistribute it and/or modify
   * it under the terms of the GNU General Public License as published by
   * the Free Software Foundation; either version 2 of the License, or
   * (at your option) any later version.
   * 
   * This program is distributed in the hope that it will be useful,
   * but WITHOUT ANY WARRANTY; without even the implied warranty of
   * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   * GNU General Public License for more details.
   *
   * You should have received a copy of the GNU General Public License
   * along with this program; if not, write to the Free Software
   * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
*/

#ifndef KRNL_LIST_H_
#define KRNL_LIST_H_

#ifndef LIST_T_
#define LIST_T_
struct list
{
	struct list *next, *prev;
};
#endif

#define init_list(l) do { \
	(l)->next = (l); (l)->prev = (l); \
	} while (0)

static __inline int is_list_empty(struct list *_lst)
{
 if(_lst && _lst->prev==_lst->next && _lst->prev==_lst)
  return 1;
 else
  return 0;
}

static __inline void list_add_t(struct list *_new, struct list *prev, struct list *next)
{
	next->prev = _new;
	_new->next = next;
	_new->prev = prev;
	prev->next = _new;
}

static __inline void list_add(struct list *_new, struct list *head)
{
	list_add_t(_new, head, head->next);
}

static __inline void list_del_t(struct list *prev, struct list *next)
{
	next->prev = prev;
	prev->next = next;
}

static __inline void list_del(struct list *item)
{
	list_del_t(item->prev, item->next);
}

#define for_each_list_entry(pos, head) \
        for (pos = (head)->next; pos != (head); pos = pos->next)

#define for_each_list_entry_reversed(pos, tail) \
        for (pos = (tail)->prev; pos != (tail); pos = pos->prev)

#define list_item(ptr, type, member) \
	((type *)((char *)(ptr)-(/*unsigned long*/char *)(&((type *)0)->member)))

#endif // KRNL_LIST_H_
