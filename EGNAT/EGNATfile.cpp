#include "EGNATfile.h"

extern double IOread;
extern double IOwrite;
extern int pageSize;
void EGNATfile::create(const char *filename) {
	if (isOpened()) return;
	fileHandle = _open(filename, O_RDWR | O_BINARY);

	if (fileHandle < 0) {
		fileHandle = _open(filename, O_BINARY | O_RDWR | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE);
	}

	setOpen(true);
	pageNum = 1;
}


void EGNATfile::open(const char *filename) {
	if (isOpened()) return;
	fileHandle = _open(filename, O_RDWR | O_BINARY);
	if (fileHandle < 0)return;
	setOpen(true);
}

void EGNATfile::close() {
	if (!isOpened()) return;
	_close(fileHandle);
	setOpen(false);
}
bool EGNATfile::isOpened() { return isOpen; }

void EGNATfile::setOpen(bool state) { isOpen = state; }


int EGNATfile::readPage(double pageNo, LeafPage *leafPage, InternalPage *internalPage) {
	int num = 0;
	char *buf = (char*)malloc(pageSize);
	memset(buf, 0, pageSize);
	if (isOpened()) {
		_lseek(fileHandle, pageNo*pageSize, SEEK_SET);
		_read(fileHandle, buf, pageSize);
		IOread++;

		int offset = 0;
		memcpy(&num, buf, sizeof(int));
		offset += sizeof(int);
		if (num < 0) {
			internalPage->num = num;
			for (int i = 0; i < internalPageObjCnt; i++) {
				memcpy(internalPage->pivots[i], buf + offset, dim*sizeof(float));
				offset += dim*sizeof(float);
			}
			for (int i = 0; i < internalPageObjCnt; i++) {
				memcpy(internalPage->max[i], buf + offset, internalPageObjCnt*sizeof(double));
				offset += internalPageObjCnt*sizeof(double);
			}
			for (int i = 0; i < internalPageObjCnt; i++) {
				memcpy(internalPage->min[i], buf + offset, internalPageObjCnt*sizeof(double));
				offset += internalPageObjCnt*sizeof(double);
			}
			memcpy(internalPage->childPageNo, buf + offset, internalPageObjCnt*sizeof(int));
		}
		else {
			leafPage->num = num;
			for (int i = 0; i < leafPageObjCnt; i++) {
				memcpy(leafPage->leaves[i], buf + offset, dim*sizeof(float));
				offset += dim*sizeof(float);
			}
			memcpy(leafPage->dist_to_parent, buf + offset, leafPageObjCnt*sizeof(double));
		}
	}
	free(buf);
	buf = NULL;
	return num;
}

void EGNATfile::writePage(double pageNo, const Page *page) {
	char *buf = (char*)malloc(pageSize);
	memset(buf, 0, pageSize);
	if (isOpened()) {
		_lseek(fileHandle, pageNo*pageSize, SEEK_SET);
		if (page->num < 0) {
			InternalPage* internalPage = (InternalPage*)page;
			int offset = 0;
			memcpy(buf, &(internalPage->num),sizeof(int));
			offset += sizeof(int);
			for (int i = 0; i < internalPageObjCnt; i++) {
				memcpy(buf + offset, internalPage->pivots[i], dim*sizeof(float));
				offset += dim*sizeof(float);
			}
			for (int i = 0; i < internalPageObjCnt; i++) {
				memcpy(buf+offset, internalPage->max[i], internalPageObjCnt*sizeof(double));
				offset += internalPageObjCnt*sizeof(double);
			}
			for (int i = 0; i < internalPageObjCnt; i++) {
				memcpy(buf + offset, internalPage->min[i], internalPageObjCnt*sizeof(double));
				offset += internalPageObjCnt*sizeof(double);
			}
			memcpy(buf + offset, internalPage->childPageNo, internalPageObjCnt*sizeof(int));

		}
		else {
			LeafPage* leafPage = (LeafPage*)page;
			int offset = 0;
			memcpy(buf, &(leafPage->num), sizeof(int));
			offset += sizeof(int) ;
			for (int i = 0; i < leafPageObjCnt; i++) {
				memcpy(buf+offset, leafPage->leaves[i], dim*sizeof(float));
				offset += dim*sizeof(float) ;
			}
			memcpy(buf + offset, leafPage->dist_to_parent, leafPageObjCnt*sizeof(double));
		}
		_write(fileHandle, buf, pageSize);
		IOwrite++;
	}
	free(buf);
	buf = NULL;
}
