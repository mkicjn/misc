//$(which tcc) $CFLAGS -run $0 "$@"; exit $?
#include <stdio.h>
#include <stdbool.h>

// "Virtual deque" - First idea: track only array indices, nothing else
struct vdeque {
	size_t head; // First populated index
	size_t tail; // First unpopulated index
	size_t pop; // Number of items in the deque
	size_t cap; // Max index in the deque array
};

static inline void vdeque_init(struct vdeque *vd, size_t array_count)
{
	// Second idea: reserve index 0 for errors
	vd->head = 1;
	vd->tail = 1;
	vd->pop  = 0;
	vd->cap  = array_count;
}

static inline bool vdeque_full(struct vdeque *vd)
{
	return vd->pop >= (vd->cap - 1);
}

static inline bool vdeque_empty(struct vdeque *vd)
{
	return vd->pop <= 0;
}

static inline size_t vdeque_succ(struct vdeque *vd, size_t n)
{
	return (n >= vd->cap - 1 ? 1 : n + 1);
}

static inline size_t vdeque_pred(struct vdeque *vd, size_t n)
{
	return (n <= 1 ? vd->cap - 1 : n - 1);
}

static inline size_t vdeque_rpush(struct vdeque *vd)
{
	if (vdeque_full(vd))
		return 0;
	size_t ret = vd->tail;
	vd->tail = vdeque_succ(vd, vd->tail);
	vd->pop++;
	return ret;
}

static inline size_t vdeque_lpop(struct vdeque *vd)
{
	if (vdeque_empty(vd))
		return 0;
	size_t ret = vd->head;
	vd->head = vdeque_succ(vd, vd->head);
	vd->pop--;
	return ret;
}

static inline size_t vdeque_lpush(struct vdeque *vd)
{
	if (vdeque_full(vd))
		return 0;
	size_t ret = vdeque_pred(vd, vd->head);
	vd->head = ret;
	vd->pop++;
	return ret;
}

static inline size_t vdeque_rpop(struct vdeque *vd)
{
	if (vdeque_empty(vd))
		return 0;
	size_t ret = vdeque_pred(vd, vd->tail);
	vd->tail = ret;
	vd->pop--;
	return ret;
}

int main()
{
	int deque[4];

#define COUNT(x)  (sizeof(x) / (sizeof((x)[0])))
	struct vdeque vd;
	vdeque_init(&vd, COUNT(deque));

	deque[0] = -1; // For easier visual identification

	printf("%lu\n", vdeque_lpop(&vd)); // 0
	printf("%lu\n", vdeque_rpop(&vd)); // 0

	deque[vdeque_rpush(&vd)] = 1;
	deque[vdeque_rpush(&vd)] = 2;
	deque[vdeque_rpush(&vd)] = 3;
	printf("%d\n", deque[vdeque_lpop(&vd)]); // 1
	printf("%d\n", deque[vdeque_lpop(&vd)]); // 2
	printf("%d\n", deque[vdeque_lpop(&vd)]); // 3

	deque[vdeque_lpush(&vd)] = 1;
	deque[vdeque_lpush(&vd)] = 2;
	deque[vdeque_lpush(&vd)] = 3;
	printf("%d\n", deque[vdeque_lpop(&vd)]); // 3
	printf("%d\n", deque[vdeque_lpop(&vd)]); // 2
	printf("%d\n", deque[vdeque_lpop(&vd)]); // 1

	deque[vdeque_lpush(&vd)] = 1;
	deque[vdeque_lpush(&vd)] = 2;
	deque[vdeque_lpush(&vd)] = 3;
	printf("%d\n", deque[vdeque_rpop(&vd)]); // 1
	printf("%d\n", deque[vdeque_rpop(&vd)]); // 2
	printf("%d\n", deque[vdeque_rpop(&vd)]); // 3

	deque[vdeque_rpush(&vd)] = 1;
	deque[vdeque_rpush(&vd)] = 2;
	deque[vdeque_rpush(&vd)] = 3;
	printf("%d\n", deque[vdeque_rpop(&vd)]); // 3
	printf("%d\n", deque[vdeque_rpop(&vd)]); // 2
	printf("%d\n", deque[vdeque_rpop(&vd)]); // 1

	deque[vdeque_rpush(&vd)] = 0;
	deque[vdeque_rpush(&vd)] = 0;
	deque[vdeque_rpush(&vd)] = 0;
	printf("%lu\n", vdeque_lpush(&vd)); // 0
	printf("%lu\n", vdeque_rpush(&vd)); // 0

	return 0;
}
