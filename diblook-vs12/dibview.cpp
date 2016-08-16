// dibview.cpp : implementation of the CDibView class
//
// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1992-1998 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#include "stdafx.h"
#include "diblook.h"

#include "dibdoc.h"
#include "dibview.h"
#include "dibapi.h"
#include "mainfrm.h"

#include "HRTimer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define BEGIN_PROCESSING() INCEPUT_PRELUCRARI()

#define END_PROCESSING(Title) SFARSIT_PRELUCRARI(Title)

#define INCEPUT_PRELUCRARI() \
	CDibDoc* pDocSrc=GetDocument();										\
	CDocTemplate* pDocTemplate=pDocSrc->GetDocTemplate();				\
	CDibDoc* pDocDest=(CDibDoc*) pDocTemplate->CreateNewDocument();		\
	BeginWaitCursor();													\
	HDIB hBmpSrc=pDocSrc->GetHDIB();									\
	HDIB hBmpDest = (HDIB)::CopyHandle((HGLOBAL)hBmpSrc);				\
	if ( hBmpDest==0 ) {												\
		pDocTemplate->RemoveDocument(pDocDest);							\
		return;															\
	}																	\
	BYTE* lpD = (BYTE*)::GlobalLock((HGLOBAL)hBmpDest);					\
	BYTE* lpS = (BYTE*)::GlobalLock((HGLOBAL)hBmpSrc);					\
	int iColors = DIBNumColors((char *)&(((LPBITMAPINFO)lpD)->bmiHeader)); \
	RGBQUAD *bmiColorsDst = ((LPBITMAPINFO)lpD)->bmiColors;	\
	RGBQUAD *bmiColorsSrc = ((LPBITMAPINFO)lpS)->bmiColors;	\
	BYTE * lpDst = (BYTE*)::FindDIBBits((LPSTR)lpD);	\
	BYTE * lpSrc = (BYTE*)::FindDIBBits((LPSTR)lpS);	\
	int dwWidth  = ::DIBWidth((LPSTR)lpS);\
	int dwHeight = ::DIBHeight((LPSTR)lpS);\
	int w=WIDTHBYTES(dwWidth*((LPBITMAPINFOHEADER)lpS)->biBitCount);	\
	HRTimer my_timer;	\
	my_timer.StartTimer();	\

#define BEGIN_SOURCE_PROCESSING \
	CDibDoc* pDocSrc=GetDocument();										\
	BeginWaitCursor();													\
	HDIB hBmpSrc=pDocSrc->GetHDIB();									\
	BYTE* lpS = (BYTE*)::GlobalLock((HGLOBAL)hBmpSrc);					\
	int iColors = DIBNumColors((char *)&(((LPBITMAPINFO)lpS)->bmiHeader)); \
	RGBQUAD *bmiColorsSrc = ((LPBITMAPINFO)lpS)->bmiColors;	\
	BYTE * lpSrc = (BYTE*)::FindDIBBits((LPSTR)lpS);	\
	int dwWidth  = ::DIBWidth((LPSTR)lpS);\
	int dwHeight = ::DIBHeight((LPSTR)lpS);\
	int w=WIDTHBYTES(dwWidth*((LPBITMAPINFOHEADER)lpS)->biBitCount);	\



#define END_SOURCE_PROCESSING	\
	::GlobalUnlock((HGLOBAL)hBmpSrc);								\
    EndWaitCursor();												\
/////////////////////////////////////////////////////////////////////////////


//---------------------------------------------------------------
#define SFARSIT_PRELUCRARI(Titlu)	\
	double elapsed_time_ms = my_timer.StopTimer();	\
	CString Title;	\
	Title.Format(_TEXT("%s - Proc. time = %.2f ms"), _TEXT(Titlu), elapsed_time_ms);	\
	::GlobalUnlock((HGLOBAL)hBmpDest);								\
	::GlobalUnlock((HGLOBAL)hBmpSrc);								\
    EndWaitCursor();												\
	pDocDest->SetHDIB(hBmpDest);									\
	pDocDest->InitDIBData();										\
	pDocDest->SetTitle((LPCTSTR)Title);									\
	CFrameWnd* pFrame=pDocTemplate->CreateNewFrame(pDocDest,NULL);	\
	pDocTemplate->InitialUpdateFrame(pFrame,pDocDest);	\

/////////////////////////////////////////////////////////////////////////////
// CDibView

IMPLEMENT_DYNCREATE(CDibView, CScrollView)

BEGIN_MESSAGE_MAP(CDibView, CScrollView)
	//{{AFX_MSG_MAP(CDibView)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnUpdateEditPaste)
	ON_MESSAGE(WM_DOREALIZE, OnDoRealize)
	//}}AFX_MSG_MAP

	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CScrollView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CScrollView::OnFilePrintPreview)
	ON_WM_LBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_COMMAND(ID_PROCESSING_GRAYSCALE, &CDibView::OnProcessingGrayscale)
	ON_COMMAND(ID_PROCESSING_CANNY, &CDibView::OnProcessingCanny)
	ON_COMMAND(ID_PROCESSING_DILATION, &CDibView::OnProcessingDilation)
	ON_COMMAND(ID_PROCESSING_EROSION, &CDibView::OnProcessingErosion)
	ON_COMMAND(ID_PROCESSING_CONNECTEDCOMPONENT, &CDibView::OnProcessingConnectedcomponent)
	ON_COMMAND(ID_PROCESSING_HOUGH, &CDibView::OnProcessingHough)
	ON_COMMAND(ID_ALGORITHM_DETECT, &CDibView::OnAlgorithmDetect)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDibView construction/destruction

CDibView::CDibView()
{
}

CDibView::~CDibView()
{
}

/////////////////////////////////////////////////////////////////////////////
// CDibView drawing

void CDibView::OnDraw(CDC* pDC)
{
	CDibDoc* pDoc = GetDocument();

	HDIB hDIB = pDoc->GetHDIB();
	if (hDIB != NULL)
	{
		LPSTR lpDIB = (LPSTR) ::GlobalLock((HGLOBAL)hDIB);
		int cxDIB = (int) ::DIBWidth(lpDIB);         // Size of DIB - x
		int cyDIB = (int) ::DIBHeight(lpDIB);        // Size of DIB - y
		::GlobalUnlock((HGLOBAL)hDIB);
		CRect rcDIB;
		rcDIB.top = rcDIB.left = 0;
		rcDIB.right = cxDIB;
		rcDIB.bottom = cyDIB;
		CRect rcDest;
		if (pDC->IsPrinting())   // printer DC
		{
			// get size of printer page (in pixels)
			int cxPage = pDC->GetDeviceCaps(HORZRES);
			int cyPage = pDC->GetDeviceCaps(VERTRES);
			// get printer pixels per inch
			int cxInch = pDC->GetDeviceCaps(LOGPIXELSX);
			int cyInch = pDC->GetDeviceCaps(LOGPIXELSY);

			//
			// Best Fit case -- create a rectangle which preserves
			// the DIB's aspect ratio, and fills the page horizontally.
			//
			// The formula in the "->bottom" field below calculates the Y
			// position of the printed bitmap, based on the size of the
			// bitmap, the width of the page, and the relative size of
			// a printed pixel (cyInch / cxInch).
			//
			rcDest.top = rcDest.left = 0;
			rcDest.bottom = (int)(((double)cyDIB * cxPage * cyInch)
				/ ((double)cxDIB * cxInch));
			rcDest.right = cxPage;
		}
		else   // not printer DC
		{
			rcDest = rcDIB;
		}
		::PaintDIB(pDC->m_hDC, &rcDest, pDoc->GetHDIB(),
			&rcDIB, pDoc->GetDocPalette());
	}
}

/////////////////////////////////////////////////////////////////////////////
// CDibView printing

BOOL CDibView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

/////////////////////////////////////////////////////////////////////////////
// CDibView commands


LRESULT CDibView::OnDoRealize(WPARAM wParam, LPARAM)
{
	ASSERT(wParam != NULL);
	CDibDoc* pDoc = GetDocument();
	if (pDoc->GetHDIB() == NULL)
		return 0L;  // must be a new document

	CPalette* pPal = pDoc->GetDocPalette();
	if (pPal != NULL)
	{
		CMainFrame* pAppFrame = (CMainFrame*)AfxGetApp()->m_pMainWnd;
		ASSERT_KINDOF(CMainFrame, pAppFrame);

		CClientDC appDC(pAppFrame);
		// All views but one should be a background palette.
		// wParam contains a handle to the active view, so the SelectPalette
		// bForceBackground flag is FALSE only if wParam == m_hWnd (this view)
		CPalette* oldPalette = appDC.SelectPalette(pPal, ((HWND)wParam) != m_hWnd);

		if (oldPalette != NULL)
		{
			UINT nColorsChanged = appDC.RealizePalette();
			if (nColorsChanged > 0)
				pDoc->UpdateAllViews(NULL);
			appDC.SelectPalette(oldPalette, TRUE);
		}
		else
		{
			TRACE0("\tSelectPalette failed in CDibView::OnPaletteChanged\n");
		}
	}

	return 0L;
}

void CDibView::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();
	ASSERT(GetDocument() != NULL);

	SetScrollSizes(MM_TEXT, GetDocument()->GetDocSize());
}


void CDibView::OnActivateView(BOOL bActivate, CView* pActivateView,
	CView* pDeactiveView)
{
	CScrollView::OnActivateView(bActivate, pActivateView, pDeactiveView);

	if (bActivate)
	{
		ASSERT(pActivateView == this);
		OnDoRealize((WPARAM)m_hWnd, 0);   // same as SendMessage(WM_DOREALIZE);
	}
}

void CDibView::OnEditCopy()
{
	CDibDoc* pDoc = GetDocument();
	// Clean clipboard of contents, and copy the DIB.

	if (OpenClipboard())
	{
		BeginWaitCursor();
		EmptyClipboard();
		SetClipboardData(CF_DIB, CopyHandle((HANDLE)pDoc->GetHDIB()));
		CloseClipboard();
		EndWaitCursor();
	}
}



void CDibView::OnUpdateEditCopy(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(GetDocument()->GetHDIB() != NULL);
}


void CDibView::OnEditPaste()
{
	HDIB hNewDIB = NULL;

	if (OpenClipboard())
	{
		BeginWaitCursor();

		hNewDIB = (HDIB)CopyHandle(::GetClipboardData(CF_DIB));

		CloseClipboard();

		if (hNewDIB != NULL)
		{
			CDibDoc* pDoc = GetDocument();
			pDoc->ReplaceHDIB(hNewDIB); // and free the old DIB
			pDoc->InitDIBData();    // set up new size & palette
			pDoc->SetModifiedFlag(TRUE);

			SetScrollSizes(MM_TEXT, pDoc->GetDocSize());
			OnDoRealize((WPARAM)m_hWnd, 0);  // realize the new palette
			pDoc->UpdateAllViews(NULL);
		}
		EndWaitCursor();
	}
}


void CDibView::OnUpdateEditPaste(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(::IsClipboardFormatAvailable(CF_DIB));
}

///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

#define PI 3.14159265358979323846264338327950288419716939937510
#define MAX_STACK_SIZE 100000
#include <queue>
using namespace std;


struct Point {
	int x;
	int y;

	bool operator == (const Point& rhs) const {
		return (x == rhs.x) && (y == rhs.y);
	}
};

struct Polar {
	float rho;
	float theta;

	bool operator == (const Polar& rhs) const {
		return (rho == rhs.rho) && (theta == rhs.theta);
	}
};

struct Line {
	Point p1;
	Point p2;

	bool operator == (const Line& rhs) const {
		return (p1 == rhs.p1) && (p2 == rhs.p2);
	}
};

DOUBLE* tabSinus;
DOUBLE* tabCosinus;
DOUBLE** tabAtan2;
DOUBLE** tabSqrt;

FLOAT* gaussianKernel;

bool computedLookupTables = false;
bool computedGaussianKernel = false;
bool computedHoughLookupTables = false;

int currentStackSize = 0;
int connectedComponentsSize = 0;
Point* stack = new Point[MAX_STACK_SIZE];

FLOAT* tabSinHough;
FLOAT* tabCosHough;

bool debugMode = false;
Point clickCoordinates = { -1 , -1 };

///////////////////////////////////////////////////////////////////////////////////

void convert2Grayscale(BYTE* image, RGBQUAD* bmiColors, int iColors, int w, int height, int width) {
	BYTE* g = new BYTE[256];

	for (int k = 0; k < iColors; k++) {
		// initialize vector g
		g[k] = (bmiColors[k].rgbRed + bmiColors[k].rgbBlue + bmiColors[k].rgbGreen) / 3;

		// „sort” the LUT
		bmiColors[k].rgbRed = bmiColors[k].rgbGreen = bmiColors[k].rgbBlue = k;
	}

	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			int k = image[i*w + j];
			image[i*w + j] = g[k];
		}
	}

	delete[] g;
}

void gaussianConvolutionKernel(FLOAT* gauss, float sigma, int maskSize)
{
	int x0 = maskSize / 2;
	int y0 = maskSize / 2;

	for (int i = 0; i < maskSize; i++)
		for (int j = 0; j < maskSize; j++) {

			gauss[i*maskSize + j] = (1.0 / (2.0 * (22.0 / 7.0) * (sigma*sigma))) * exp(-(float)((i - x0)*(i - x0) + (j - y0)*(j - y0)) / (2 * sigma*sigma));
		}
}

void gaussianFilter(BYTE* input, BYTE* output, int w, int height, int width, float* gauss, int maskSize)
{
	float sum = 0;
	for (int i = 0; i < w*height; i++)
		output[i] = input[i];

	for (int i = maskSize / 2; i < height - maskSize / 2; i++) {
		for (int j = maskSize / 2; j < width - maskSize / 2; j++)
		{
			sum = 0;

			for (int k = 0; k < maskSize; k++) {
				for (int l = 0; l < maskSize; l++) {
					sum += input[(i + k - maskSize / 2)*w + j + l - maskSize / 2] * gauss[k*maskSize + l];
				}
			}

			output[i*w + j] = sum;
		}
	}
}

void gradientConvolution(BYTE* input, float* auxX, float* auxY, int height, int width, int w)
{
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			auxX[i*w + j] = 0;
			auxY[i*w + j] = 0;
		}
	}

	float hX[3][3] = {
		{ -1, 0, 1 },
		{ -2, 0, 2 },
		{ -1, 0, 1 }
	};

	float hY[3][3] = {
		{ 1, 2, 1 },
		{ 0, 0, 0 },
		{ -1, -2, -1 }
	};

	float partSumX = 0;
	float partSumY = 0;

	for (int i = 1; i < height - 1; i++) {
		for (int j = 1; j < width - 1; j++) {
			partSumX += hX[0][0] * input[(i + 1)*w + (j - 1)];
			partSumX += hX[0][1] * input[(i + 1)*w + j];
			partSumX += hX[0][2] * input[(i + 1)*w + (j + 1)];

			partSumX += hX[1][0] * input[i*w + (j - 1)];
			partSumX += hX[1][1] * input[i*w + j];
			partSumX += hX[1][2] * input[i*w + (j + 1)];

			partSumX += hX[2][0] * input[(i - 1)*w + (j - 1)];
			partSumX += hX[2][1] * input[(i - 1)*w + j];
			partSumX += hX[2][2] * input[(i - 1)*w + (j + 1)];

			partSumY += hY[0][0] * input[(i + 1)*w + (j - 1)];
			partSumY += hY[0][1] * input[(i + 1)*w + j];
			partSumY += hY[0][2] * input[(i + 1)*w + (j + 1)];

			partSumY += hY[1][0] * input[i*w + (j - 1)];
			partSumY += hY[1][1] * input[i*w + j];
			partSumY += hY[1][2] * input[i*w + (j + 1)];

			partSumY += hY[2][0] * input[(i - 1)*w + (j - 1)];
			partSumY += hY[2][1] * input[(i - 1)*w + j];
			partSumY += hY[2][2] * input[(i - 1)*w + (j + 1)];

			auxX[i*w + j] = partSumX / 4;
			auxY[i*w + j] = partSumY / 4;

			partSumX = 0;
			partSumY = 0;
		}
	}
}

void gradientMagnitude(float *magnitude, float *x, float *y, int height, int width, int w)
{
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			//magnitude[i*w + j] = tabSqrt[(int)x[i*w + j] + 255][(int)y[i*w + j] + 255];
			magnitude[i*w + j] = sqrt(x[i*w + j] * x[i*w + j] + y[i*w + j] * y[i*w + j]);
		}
	}
}

void gradientDirection(float* direction, float *x, float *y, int height, int width, int w)
{
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			//direction[i*w + j] = tabAtan2[(int)y[i*w + j] + 255][(int)x[i*w + j] + 255];
			direction[i*w + j] = atan2(y[i*w + j], x[i*w + j]);
		}
	}
}

void edgeLinking(BYTE* input, BYTE* output, int height, int width, int w)
{
	int zeroGradient = 0;
	int edgePixels = 0;
	float p = 0.3;	//0.3

	int* histogram = new int[width];

	for (int i = 0; i < height; i++)
		histogram[i] = 0;

	for (int i = 1; i < height - 1; i++) {
		for (int j = 1; j < width - 1; j++) {
			if (input[i*w + j] == 0)
				zeroGradient++;

			histogram[input[i*w + j]]++;
		}
	}

	int noNonEdge = (1 - p)*(height*width - histogram[0]);

	int sum = 0;
	int hardEdgeTh = 0;
	do
	{
		hardEdgeTh++;
		sum += histogram[hardEdgeTh];
	} while (sum < noNonEdge);

	int softEdgeTh = hardEdgeTh * 0.4;

	queue<Point> que;

	for (int i = 0; i < height; i++) {
		output[i*w] = output[i*w + width - 1] = 0;
	}
	for (int j = 0; j < width; j++) {
		output[j] = output[(height - 1)*w + j] = 0;
	}

	Point point;
	for (int i = 1; i < height - 1; i++) {
		for (int j = 1; j < width - 1; j++) {
			if (input[i*w + j] < softEdgeTh)
				output[i*w + j] = 0;
			else
				if (input[i*w + j] < hardEdgeTh)
					output[i*w + j] = 150;
				else
				{
					output[i*w + j] = 255;
					point = { i, j };

					if (point.x > 0 && point.x < width && point.y > 0 && point.y < height)
						que.push(point);
				}
		}
	}

	Point aux;
	while (!que.empty())
	{
		point = que.front();
		que.pop();

		if (output[point.x*w + point.y + 1] == 150)
		{
			output[point.x*w + point.y + 1] = 255;
			aux.x = point.x;
			aux.y = point.y + 1;
			que.push(aux);
		}
		if (output[point.x*w + point.y - 1] == 150)
		{
			output[point.x*w + point.y - 1] = 255;
			aux.x = point.x;
			aux.y = point.y - 1;
			que.push(aux);
		}
		if (output[(point.x + 1)*w + point.y + 1] == 150)
		{
			output[(point.x + 1)*w + point.y + 1] = 255;
			aux.x = point.x + 1;
			aux.y = point.y + 1;
			que.push(aux);
		}
		if (output[(point.x + 1)*w + point.y - 1] == 150)
		{
			output[(point.x + 1)*w + point.y - 1] = 255;
			aux.x = point.x + 1;
			aux.y = point.y - 1;
			que.push(aux);
		}
		if (output[(point.x - 1)*w + point.y + 1] == 150)
		{
			output[(point.x - 1)*w + point.y + 1] = 255;
			aux.x = point.x - 1;
			aux.y = point.y + 1;
			que.push(aux);
		}
		if (output[(point.x - 1)*w + point.y - 1] == 150)
		{
			output[(point.x - 1)*w + point.y - 1] = 255;
			aux.x = point.x - 1;
			aux.y = point.y - 1;
			que.push(aux);
		}
		if (output[(point.x + 1)*w + point.y] == 150)
		{
			output[(point.x + 1)*w + point.y] = 255;
			aux.x = point.x + 1;
			aux.y = point.y;
			que.push(aux);
		}
		if (output[(point.x - 1)*w + point.y] == 150)
		{
			output[(point.x - 1)*w + point.y] = 255;
			aux.x = point.x - 1;
			aux.y = point.y;
			que.push(aux);
		}
	}

	for (int i = 1; i < height - 1; i++) {
		for (int j = 1; j < width - 1; j++) {
			if (output[i*w + j] == 150)
				output[i*w + j] = 0;
		}
	}

	delete[] histogram;
	queue<Point>().swap(que);
}

void nonMaximaSuppression(float* result, float *magnitude, float *direction, int height, int width, int w)
{
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			direction[i*w + j] += PI;

			if (direction[i*w + j] >= 0 && direction[i*w + j] < PI / 8 || direction[i*w + j] >= 7.0*PI / 8 && direction[i*w + j] < 9.0*PI / 8 || direction[i*w + j] >= 15.0*PI / 8 && direction[i*w + j] < 2 * PI)
			{
				if (magnitude[i*w + j] > magnitude[i*w + j - 1] && magnitude[i*w + j] > magnitude[i*w + j + 1])
					result[i*w + j] = magnitude[i*w + j];
				else
					result[i*w + j] = 0;
			}
			else if (direction[i*w + j] >= PI / 8 && direction[i*w + j] < 3.0*PI / 8 || direction[i*w + j] >= 9.0*PI / 8 && direction[i*w + j] < 11.0*PI / 8)
			{
				if (magnitude[i*w + j] > magnitude[(i + 1)*w + j + 1] && magnitude[i*w + j] > magnitude[(i - 1)*w + j - 1])
					result[i*w + j] = magnitude[i*w + j];
				else
					result[i*w + j] = 0;
			}
			else if (direction[i*w + j] >= 3.0*PI / 8 && direction[i*w + j] < 5.0*PI / 8 || direction[i*w + j] >= 11.0*PI / 8 && direction[i*w + j] < 13.0*PI / 8)
			{
				if (magnitude[i*w + j] > magnitude[(i + 1)*w + j] && magnitude[i*w + j] > magnitude[(i - 1)*w + j])
					result[i*w + j] = magnitude[i*w + j];
				else
					result[i*w + j] = 0;
			}
			else if (direction[i*w + j] >= 5.0*PI / 8 && direction[i*w + j] < 7.0*PI / 8 || direction[i*w + j] >= 13.0*PI / 8 && direction[i*w + j] < 15.0*PI / 8)
			{
				if (magnitude[i*w + j] > magnitude[(i + 1)*w + j - 1] && magnitude[i*w + j] > magnitude[(i - 1)*w + j + 1])
					result[i*w + j] = magnitude[i*w + j];
				else
					result[i*w + j] = 0;
			}
			else
				result[i*w + j] = 0;
		}
	}
}

void computeLookUpTables() {
	tabCosinus = new DOUBLE[360];
	tabSinus = new DOUBLE[360];
	tabAtan2 = new DOUBLE*[255 * 8 + 1];
	tabSqrt = new DOUBLE*[255 * 8 + 1];

	for (int i = 0; i < 255 * 8 + 1; i++)
		tabAtan2[i] = new DOUBLE[255 * 8 + 1];

	for (int i = 0; i < 255 * 8 + 1; i++)
		tabSqrt[i] = new DOUBLE[255 * 8 + 1];

	for (int th = 0; th < 360; th++) {
		double radian = th * PI / 180;
		tabSinus[th] = sin(radian);
		tabCosinus[th] = cos(radian);
	}

	for (int i = 0; i < 255 * 8 + 1; i++) {
		for (int j = 0; j < 255 * 8 + 1; j++) {
			tabAtan2[i][j] = atan2(i - 255, j - 255);
		}
	}

	for (int i = 0; i < 255 * 8 + 1; i++) {
		for (int j = 0; j < 255 * 8 + 1; j++) {
			tabSqrt[i][j] = sqrt((i - 255)*(i - 255) + (j - 255)*(j - 255));
		}
	}

	computedLookupTables = true;
}

void computeGaussianKernel(int kernelSize, int sigma) {
	gaussianKernel = new FLOAT[kernelSize*kernelSize];
	gaussianConvolutionKernel(gaussianKernel, sigma, kernelSize);

	computedGaussianKernel = true;
}

void cannyEdgeDetection(BYTE* input, int height, int width, int w) {
	int kernelSize = 5;
	float sigma = 1.3;

	if (!computedGaussianKernel)
		computeGaussianKernel(kernelSize, sigma);

	BYTE* gaussian = new BYTE[height * w];
	gaussianFilter(input, gaussian, w, height, width, gaussianKernel, kernelSize);

	float* convolutionX = new float[height * w];
	float* convolutionY = new float[height * w];
	gradientConvolution(gaussian, convolutionX, convolutionY, height, width, w);

	float* magnitude = new float[height * w];
	gradientMagnitude(magnitude, convolutionX, convolutionY, height, width, w);

	float* direction = new float[height * w];
	gradientDirection(direction, convolutionX, convolutionY, height, width, w);

	float* supression = new float[height * w];
	nonMaximaSuppression(supression, magnitude, direction, height, width, w);

	double max = 0;
	double min = 255;
	for (int i = 1; i < height - 1; i++) {
		for (int j = 1; j < width - 1; j++) {
			if (max < supression[i*w + j])
				max = supression[i*w + j];
			if (min > supression[i*w + j])
				min = supression[i*w + j];
		}
	}

	float ratio = 255.0 / (max - min);
	for (int i = 1; i < height - 1; i++)
		for (int j = 1; j < width - 1; j++)
			input[i*w + j] = supression[i*w + j] / sqrt(2.0);

	BYTE* thresholded = new BYTE[height * w];
	edgeLinking(input, thresholded, height, width, w);

	for (int i = 0; i < height; i++)
		for (int j = 0; j < width; j++)
			input[i*w + j] = thresholded[i*w + j];

	delete[] gaussian;
	delete[] convolutionX;
	delete[] convolutionY;
	delete[] magnitude;
	delete[] direction;
	delete[] supression;
	delete[] thresholded;
}

void morphologicalDilation(BYTE* input, int height, int width) {
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			if (input[i*width + j] == 255) {
				//CROSS
				if (i > 0 && input[(i - 1)*width + j] == 0)
					input[(i - 1)*width + j] = 128;
				if (j > 0 && input[i*width + (j - 1)] == 0)
					input[i*width + (j - 1)] = 128;
				if (i + 1 < height && input[(i + 1)*width + j] == 0)
					input[(i + 1)*width + j] = 128;
				if (j + 1 < width && input[i*width + (j + 1)] == 0)
					input[i*width + (j + 1)] = 128;
				//SQUARE
				if (i > 0 && input[(i - 1)*width + (j - 1)] == 0)
					input[(i - 1)*width + (j - 1)] = 128;
				if (j > 0 && input[(i + 1)*width + (j - 1)] == 0)
					input[(i + 1)*width + (j - 1)] = 128;
				if (i + 1 < height && input[(i + 1)*width + (j + 1)] == 0)
					input[(i + 1)*width + (j + 1)] = 128;
				if (j + 1 < width && input[(i - 1)*width + j + 1] == 0)
					input[(i - 1)*width + j + 1] = 128;
			}
		}
	}

	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			if (input[i *width + j] == 128) {
				input[i*width + j] = 255;
			}
		}
	}
}

void morphologicalErosion(BYTE* input, int height, int width) {
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {

			if (input[i*width + j] == 255) {
				//CROSS
				if (i > 0 && j > 0 && i + 1 < height && j + 1 < width) {
					if (input[(i - 1)*width + j] == 255 || input[(i - 1)*width + j] == 128)
						if (input[i*width + (j - 1)] == 255 || input[i*width + (j - 1)] == 128)
							if (input[(i + 1)*width + j] == 255 || input[(i + 1)*width + j] == 128)
								if (input[i*width + (j + 1)] == 255 || input[i*width + (j + 1)] == 128)
									continue;

					input[i*width + j] = 128;
				}
			}
		}
	}

	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			if (input[i *width + j] == 128) {
				input[i*width + j] = 0;
			}
		}
	}
}

void stack_push(Point coord)
{
	if (currentStackSize < MAX_STACK_SIZE - 2)
	{
		stack[currentStackSize + 1] = coord;
		currentStackSize++;
	}
	else
	{
		exit(1);
	}
}

Point stack_pop()
{
	if (currentStackSize > 0)
	{
		Point returned;
		returned.x = stack[currentStackSize].x;
		returned.y = stack[currentStackSize].y;
		currentStackSize--;
		return returned;
	}
	else
	{
		exit(2);
	}
}

void connectedComponentLabeling(BYTE* img, int height, int width)
{
	int minComponentThreshold = 30;
	int maxComponentThreshold = 700;
	int fillRatioPercentage = 40;

	currentStackSize = 0;
	connectedComponentsSize = 0;

	Point* currentComponent = new Point[MAX_STACK_SIZE];
	int* label = new int[width * height];

	int lab = 1;
	int currentComponentPixelCount = 0;
	int currentColor = 0;

	for (int i = 0; i < width * height; i++)
		label[i] = 0;

	for (int r = 1; r < height - 1; r++) {
		for (int c = 1; c < width - 1; c++) {
			if (img[r * width + c] <= 0)
				continue;

			if (label[r * width + c] > 0)
				continue;

			Point pos;
			pos.x = r;
			pos.y = c;
			stack_push(pos);

			currentColor = img[r * width + c];
			label[r * width + c] = lab;

			int xMin = height, xMax = 0, yMin = width, yMax = 0;

			while (currentStackSize != 0)
			{
				Point current = stack_pop();
				currentComponent[currentComponentPixelCount] = current;
				currentComponentPixelCount++;

				int i = current.x;
				i = i > (height - 2) ? (height - 2) : i;
				i = i < 1 ? 1 : i;

				int j = current.y;
				j = j > (width - 2) ? (width - 2) : j;
				j = j < 1 ? 1 : j;

				if (i > xMax)
					xMax = i;
				if (i < xMin)
					xMin = i;
				if (j > yMax)
					yMax = j;
				if (j < yMin)
					yMin = j;

				if (img[(i - 1) * width + j - 1] == currentColor && label[(i - 1) * width + j - 1] == 0)
				{
					Point neighbour;
					neighbour.x = i - 1;
					neighbour.y = j - 1;
					stack_push(neighbour);
					label[(i - 1) * width + j - 1] = lab;
				}
				if (img[(i - 1) * width + j] == currentColor && label[(i - 1) * width + j] == 0)
				{
					Point neighbour;
					neighbour.x = i - 1;
					neighbour.y = j;
					stack_push(neighbour);
					label[(i - 1) * width + j] = lab;
				}
				if (img[(i - 1) * width + j + 1] == currentColor && label[(i - 1) * width + j + 1] == 0)
				{
					Point neighbour;
					neighbour.x = i - 1;
					neighbour.y = j + 1;
					stack_push(neighbour);
					label[(i - 1) * width + j + 1] = lab;
				}
				if (img[i * width + j - 1] == currentColor && label[i * width + j - 1] == 0)
				{
					Point neighbour;
					neighbour.x = i;
					neighbour.y = j - 1;
					stack_push(neighbour);
					label[i * width + j - 1] = lab;
				}
				if (img[i * width + j + 1] == currentColor && label[i * width + j + 1] == 0)
				{
					Point neighbour;
					neighbour.x = i;
					neighbour.y = j + 1;
					stack_push(neighbour);
					label[i * width + j + 1] = lab;
				}
				if (img[(i + 1) * width + j - 1] == currentColor && label[(i + 1) * width + j - 1] == 0)
				{
					Point neighbour;
					neighbour.x = i + 1;
					neighbour.y = j - 1;
					stack_push(neighbour);
					label[(i + 1) * width + j - 1] = lab;
				}
				if (img[(i + 1) * width + j] == currentColor && label[(i + 1) * width + j] == 0)
				{
					Point neighbour;
					neighbour.x = i + 1;
					neighbour.y = j;
					stack_push(neighbour);
					label[(i + 1) * width + j] = lab;
				}
				if (img[(i + 1) * width + j + 1] == currentColor && label[(i + 1) * width + j + 1] == 0)
				{
					Point neighbour;
					neighbour.x = i + 1;
					neighbour.y = j + 1;
					stack_push(neighbour);
					label[(i + 1) * width + j + 1] = lab;
				}

			}

			if (currentComponentPixelCount < minComponentThreshold || currentComponentPixelCount > maxComponentThreshold) {
				for (int i = 0; i < currentComponentPixelCount; i++) {
					Point current = currentComponent[i];
					img[current.x * width + current.y] = 0;
				}
			}
			else {
				int areaHeight = xMax - xMin > 0 ? xMax - xMin : 1;
				int areaWidth = yMax - yMin > 0 ? yMax - yMin : 1;
				int fillingRatio = (currentComponentPixelCount * 100) / (areaHeight * areaWidth);

				if (fillingRatio > fillRatioPercentage) {
					for (int i = 0; i < currentComponentPixelCount; i++) {
						Point current = currentComponent[i];
						img[current.x * width + current.y] = 0;
					}
				}
			}

			currentComponentPixelCount = 0;
			lab++;
		}
	}

	delete[] label;
	delete[] currentComponent;
}

void computeHoughLookupTables(int numangle, float theta, float irho) {

	tabSinHough = new float[numangle];
	tabCosHough = new float[numangle];
	float ang = 0;
	for (int n = 0; n < numangle; ang += theta, n++)
	{
		tabSinHough[n] = (float)(sin((double)ang) * irho);
		tabCosHough[n] = (float)(cos((double)ang) * irho);
	}

	computedHoughLookupTables = true;
}


void houghLineTransform(BYTE* input, vector<Line>* lines, int height, int width, int w) {
	float resolution_ro = 1; //1 pixel;
	float resolution_theta = PI / 180; //1 degree;
	int threshold = 30;	//minimum line length;
	double min_theta = 0;
	double max_theta = PI;

	float irho = 1 / resolution_ro;
	int numangle = round(PI / resolution_theta);
	int numrho = round(((width + height) * 2 + 1) / resolution_ro);

	if (!computedHoughLookupTables)
		computeHoughLookupTables(numangle, resolution_theta, irho);

	int* accum = new int[(numangle + 2) * (numrho + 2)];
	memset(accum, 0, sizeof(accum[0]) * (numangle + 2) * (numrho + 2));

	for (int i = 0; i < height; i++)
		for (int j = 0; j < width; j++)
		{
			if (input[i * w + j] != 0)
				for (int n = 0; n < numangle; n++)
				{
					int r = round(j * tabCosHough[n] + i * tabSinHough[n]);
					r += (numrho - 1) / 2;

					if (r >= 0)
						accum[(n + 1) * (numrho + 2) + r + 1]++;
				}
		}

	vector<Polar> polarLines;
	int window = 20;
	for (int r = 0; r < numrho; r++)
		for (int n = 0; n < numangle; n++) {

			int base = (n + 1) * (numrho + 2) + r + 1;
			if (accum[base] >= threshold) {
				bool max = true;

				for (int i = (n - window / 2 > 0 ? n - window / 2 : 0); i <= (n + window / 2 < numangle ? n + window / 2 : numangle - 1); i++) {
					for (int j = (r - window / 2 >= 0 ? r - window / 2 : 0); j <= (r + window / 2 < numrho ? r + window / 2 : numrho - 1); j++) {
						int base2 = (i + 1) * (numrho + 2) + j + 1;

						if (accum[base2] > accum[base])
							max = false;
					}
				}

				if (max) {
					double scale = 1.0 / (numrho + 2);
					int n1 = floor(base*scale) - 1;
					int r1 = base - (n1 + 1)*(numrho + 2) - 1;
					float ro = (r1 - (numrho - 1)*0.5f) * resolution_ro;
					float theta = n1 * resolution_theta;

					polarLines.push_back({ ro, theta });
				}
			}
		}

	Polar polar;
	Point p1, p2;
	for (int i = 0; i < polarLines.size(); i++) {
		polar = polarLines[i];

		double cos_t = cos(polar.theta);
		double sin_t = sin(polar.theta);

		double x0 = cos_t * polar.rho;
		double y0 = sin_t * polar.rho;

		int alpha = 1000;
		int x1 = round(x0 + 2 * alpha * -sin_t);
		int y1 = round(y0 + 2 * alpha * cos_t);
		p1 = { x1,y1 };

		int x2 = round(x0 - 2 * alpha * -sin_t);
		int y2 = round(y0 - 2 * alpha * cos_t);
		p2 = { x2,y2 };

		lines->push_back({ p1,p2 });
	}

	delete[] accum;
	vector<Polar>().swap(polarLines);
}

bool lineIntersection(Line line1, Line line2, Point* inters) {
	float a = line1.p1.x - line1.p2.x;
	float b = line2.p1.y - line2.p2.y;
	float c = line1.p1.y - line1.p2.y;
	float d = line2.p1.x - line2.p2.x;

	float det = a * b - c * d;
	float epsilon = 1e-6;

	if (fabs(det) < epsilon)
		return false;

	float pre = (line1.p1.x*line1.p2.y - line1.p1.y*line1.p2.x);
	float post = (line2.p1.x*line2.p2.y - line2.p1.y*line2.p2.x);

	inters->x = (pre * d - a * post) / det;
	inters->y = (pre * b - c * post) / det;

	return true;
}

float point2LineDistance(Point point, Line line) {
	int a = point.x - line.p1.x;
	int b = point.y - line.p1.y;
	int c = line.p2.x - line.p1.x;
	int d = line.p2.y - line.p1.y;

	return abs(a * d - c * b) / sqrt(c * c + d * d);
}

float point2PointDistance(Point p1, Point p2) {
	float diffY = p1.y - p2.y;
	float diffX = p1.x - p2.x;

	return sqrt((diffY * diffY) + (diffX * diffX));
}

void vanishingLinesExtractionRansac(vector<Line>* lines, vector<Line>* vanishingLines, Point* vanishingPoint, int width, int height) {
	int radiusSize = 10;

	//Ransac parameters
	float p = 0.99;
	float w = 0.1;
	float e = 0.7;
	int n = round(log(1 - p) / log(1 - w*w));

	vector<Line> finalInters;
	vector<Line> tempInters;

	int max = 0;
	int index1, index2;
	Line line1, line2, line3;
	Point inters;
	int size = lines->size();

	if (size == 0)
		return;

	for (int i = 0; i < n; i++) {
		index1 = rand() % size;
		index2 = rand() % size;

		while (index1 == index2)
			index2 = rand() % size;

		line1 = lines->at(index1);
		line2 = lines->at(index2);

		if (lineIntersection(line1, line2, &inters)) {
			if ((inters.x >= 0 && inters.x < width) && (inters.y > 0 && inters.y < height)) {
				for (int j = 0; j < size; j++) {
					if (i == j)
						continue;

					line3 = lines->at(j);

					if (point2LineDistance(inters, line3) <= radiusSize) {
						tempInters.push_back(line3);
					}
				}

				if (tempInters.size() > max) {
					max = tempInters.size();
					*vanishingPoint = inters;
					finalInters = tempInters;
					tempInters = vector<Line>();
				}
				else

					tempInters = vector<Line>();
			}
		}
	}

	*vanishingLines = finalInters;

	vector<Line>().swap(tempInters);
	vector<Line>().swap(finalInters);
}

bool sizeSortFunction(const pair<Point, vector<Line>> pair1, const pair<Point, vector<Line>> pair2) {
	if (pair1.second.size() > pair2.second.size())
		return true;
	else
		return false;
}

void vanishingLinesExtractionRansacMultiple(vector<Line>* lines, vector<pair<Point, vector<Line>>>* pairs, int height, int width) {
	int radiusSize = 20;
	int clusterDistance = 30;
	int elements = 3;

	//Ransac parameters
	float p = 0.99;
	float w = 0.1;
	float e = 0.7;
	int n = round(log(1 - p) / log(1 - w*w));

	vector<pair<Point, vector<Line>>> temps;
	vector<Line> tempInters;

	vector<pair<Point, vector<Line>>> finalPairs;

	int index1, index2;
	Line line1, line2, line3;
	Point inters;
	int size = lines->size();

	if (size == 0)
		return;

	for (int i = 0; i < n; i++) {
		index1 = rand() % size;
		index2 = rand() % size;

		while (index1 == index2)
			index2 = rand() % size;

		line1 = lines->at(index1);
		line2 = lines->at(index2);

		if (lineIntersection(line1, line2, &inters)) {
			if ((inters.x >= 0 && inters.x < width) && (inters.y > 0 && inters.y < height)) {

				for (int j = 0; j < size; j++) {
					line3 = lines->at(j);

					if (point2LineDistance(inters, line3) < radiusSize) {
						tempInters.push_back(line3);
					}
				}

				temps.push_back(make_pair(inters, tempInters));
				tempInters = vector<Line>();
			}
		}
	}

	sort(temps.begin(), temps.end(), sizeSortFunction);

	int m = min(temps.size(), elements);
	Point center;
	Point current;
	bool nearTo = false;
	for (int i = 0; i < temps.size(); i++) {
		if (finalPairs.size() < m) {
			if (finalPairs.empty()) {
				finalPairs.push_back(temps[0]);
			}
			else {
				center = temps[i].first;

				for (int j = 0; j < finalPairs.size(); j++) {
					current = finalPairs[j].first;

					if (point2PointDistance(center, current) < clusterDistance) {
						nearTo = true;
					}
				}

				if (!nearTo)
					finalPairs.push_back(temps[i]);
				else
					nearTo = false;
			}
		}
		else {
			break;
		}
	}

	*pairs = finalPairs;

	vector<Line>().swap(tempInters);
	vector<pair<Point, vector<Line>>>().swap(temps);
	vector<pair<Point, vector<Line>>>().swap(finalPairs);
}

void vanishingLinesExtractionMultiple(vector<Line>* lines, vector<pair<Point, vector<Line>>>* pairs, int height, int width)
{
	int radiusSize = 20;
	int clusterDistance = 20;
	int minimumInters = 6; // 3 lines;
	int elements = 1;

	vector<pair<Point, vector<Line>>> temps;
	vector<Line> tempInters;

	vector<pair<Point, vector<Line>>> finalPairs;

	int index1, index2;
	Line line1, line2, line3;
	Point inters;
	int size = lines->size();

	if (size == 0)
		return;

	for (int i = 0; i < size; i++)
	{
		for (int j = i; j < size; j++)
		{
			if (i == j)
				continue;

			line1 = lines->at(i);
			line2 = lines->at(j);

			if (lineIntersection(line1, line2, &inters))
			{
				if ((inters.x >= 0 && inters.x < width) && (inters.y > 0 && inters.y < height))
				{
					for (int k = 0; k < size; k++) {

						if (k == i || k == j)
							continue;

						line3 = lines->at(k);

						if (point2LineDistance(inters, line3) < radiusSize)
							tempInters.push_back(line3);
					}

					if (tempInters.size() >= minimumInters)
						temps.push_back(make_pair(inters, tempInters));

					tempInters = vector<Line>();
				}
			}
		}
	}

	sort(temps.begin(), temps.end(), sizeSortFunction);

	int m = min(temps.size(), elements);
	Point center;
	Point current;
	bool nearTo = false;
	for (int i = 0; i < temps.size(); i++) {
		if (finalPairs.size() < m) {
			if (finalPairs.empty()) {
				finalPairs.push_back(temps[0]);
			}
			else {
				center = temps[i].first;

				for (int j = 0; j < finalPairs.size(); j++) {
					current = finalPairs[j].first;

					if (point2PointDistance(center, current) < clusterDistance) {
						nearTo = true;
					}
				}

				if (!nearTo)
					finalPairs.push_back(temps[i]);
				else
					nearTo = false;
			}
		}
		else {
			break;
		}
	}

	*pairs = finalPairs;

	vector<Line>().swap(tempInters);
	vector<pair<Point, vector<Line>>>().swap(temps);
	vector<pair<Point, vector<Line>>>().swap(finalPairs);
}

void CDibView::OnProcessingGrayscale()
{
	BEGIN_PROCESSING();

	convert2Grayscale(lpDst, bmiColorsDst, iColors, w, dwHeight, dwWidth);

	END_PROCESSING("Grayscale");

}


void CDibView::OnProcessingCanny()
{
	BEGIN_PROCESSING();

	if (!computedLookupTables)
		computeLookUpTables();

	int height = (clickCoordinates.x != -1 && clickCoordinates.y != -1) ? clickCoordinates.y : dwHeight;
	cannyEdgeDetection(lpDst, height, dwWidth, w);

	END_PROCESSING("Canny");
}


void CDibView::OnProcessingDilation()
{
	BEGIN_PROCESSING();

	int height = (clickCoordinates.x != -1 && clickCoordinates.y != -1) ? clickCoordinates.y : dwHeight;
	morphologicalDilation(lpDst, height, dwWidth);

	END_PROCESSING("Dilation");
}


void CDibView::OnProcessingErosion()
{
	BEGIN_PROCESSING();

	int height = (clickCoordinates.x != -1 && clickCoordinates.y != -1) ? clickCoordinates.y : dwHeight;
	morphologicalErosion(lpDst, height, dwWidth);

	END_PROCESSING("Erosion");
}

#define MIN_COMPONENT_THRESHOLD 30
#define MAX_COMPONENT_THRESHOLD 700
#define FILL_RATIO_PERCENTAGE 40
Point* currentComponent = NULL;
int* label = NULL;

void connectedComponentLabeling(BYTE* image, int height, int width, int w, int horizonLineRow) {
	currentStackSize = 0;
	connectedComponentsSize = 0;

	if (currentComponent == NULL)
		currentComponent = new Point[width * height];

	if (label == NULL)
		label = new int[width * height];

	int labelValue = 1;
	int componentPixels = 0;
	int currentColor;

	for (int i = 0; i < width * height; i++)
		label[i] = 0;

	int xMin, xMax, yMin, yMax;

	for (int i = (horizonLineRow + 1); i < height - 1; i++) {
		for (int j = 1; j < width - 1; j++) {

			if (image[i * w + j] <= 0)
				continue;

			if (label[i * w + j] > 0)
				continue;

			Point point;
			point.x = i;
			point.y = j;
			stack_push(point);

			currentColor = image[i * w + j];
			label[i * w + j] = labelValue;

			int xMin = height;
			int xMax = 0;
			int yMin = width;
			int yMax = 0;

			while (currentStackSize != 0) {
				Point current;
				current = stack_pop();

				currentComponent[componentPixels] = current;
				componentPixels++;

				current.x = current.x > (height - 2) ? (height - 2) : current.x;
				current.x = current.x < 2 ? 2 : current.x;
				current.y = current.y > (width - 2) ? (width - 2) : current.y;
				current.y = current.y < 2 ? 2 : current.y;

				if (current.x > xMax)
					xMax = current.x;
				if (current.x < xMin)
					xMin = current.x;
				if (current.y > yMax)
					yMax = current.y;
				if (current.y < yMin)
					yMin = current.y;

				if (image[(current.x - 1) * w + current.y - 1] == currentColor && label[(current.x - 1) * w + current.y - 1] == 0)
				{
					Point neighbour;
					neighbour.x = current.x - 1;
					neighbour.y = current.y - 1;
					stack_push(neighbour);

					label[(current.x - 1) * w + current.y - 1] = labelValue;
				}
				if (image[(current.x - 1) * w + current.y] == currentColor && label[(current.x - 1) * w + current.y] == 0)
				{
					Point neighbour;
					neighbour.x = current.x - 1;
					neighbour.y = current.y;
					stack_push(neighbour);

					label[(current.x - 1) * w + current.y] = labelValue;
				}
				if (image[(current.x - 1) * w + current.y + 1] == currentColor && label[(current.x - 1) * w + current.y + 1] == 0)
				{
					Point neighbour;
					neighbour.x = current.x - 1;
					neighbour.y = current.y + 1;
					stack_push(neighbour);

					label[(current.x - 1) * w + current.y + 1] = labelValue;
				}
				if (image[current.x * w + current.y - 1] == currentColor && label[current.x * w + current.y - 1] == 0)
				{
					Point neighbour;
					neighbour.x = current.x;
					neighbour.y = current.y - 1;
					stack_push(neighbour);

					label[current.x * w + current.y - 1] = labelValue;
				}
				if (image[current.x * w + current.y + 1] == currentColor && label[current.x * w + current.y + 1] == 0)
				{
					Point neighbour;
					neighbour.x = current.x;
					neighbour.y = current.y + 1;
					stack_push(neighbour);

					label[current.x * w + current.y + 1] = labelValue;
				}
				if (image[(current.x + 1) * w + current.y - 1] == currentColor && label[(current.x + 1) * w + current.y - 1] == 0)
				{
					Point neighbour;
					neighbour.x = current.x + 1;
					neighbour.y = current.y - 1;
					stack_push(neighbour);

					label[(current.x + 1) * w + current.y - 1] = labelValue;
				}
				if (image[(current.x + 1) * w + current.y] == currentColor && label[(current.x + 1) * w + current.y] == 0)
				{
					Point neighbour;
					neighbour.x = current.x + 1;
					neighbour.y = current.y;
					stack_push(neighbour);

					label[(current.x + 1) * w + current.y] = labelValue;
				}
				if (image[(current.x + 1) * w + current.y + 1] == currentColor && label[(current.x + 1) * w + current.y + 1] == 0)
				{
					Point neighbour;
					neighbour.x = current.x + 1;
					neighbour.y = current.y + 1;
					stack_push(neighbour);

					label[(current.x + 1) * w + current.y + 1] = labelValue;
				}

			}

			//Component filtering: size & fill-ratio
			if (componentPixels < MIN_COMPONENT_THRESHOLD || componentPixels > MAX_COMPONENT_THRESHOLD) {
				for (int m = 0; m < componentPixels; m++) {
					Point aux;
					aux = currentComponent[m];
					image[aux.x * w + aux.y] = 0;
				}
			}
			else {
				int areaHeight = xMax - xMin > 0 ? xMax - xMin : 1;
				int areaWidth = yMax - yMin > 0 ? yMax - yMin : 1;
				int fillingRatio = (componentPixels * 100) / (areaHeight * areaWidth);

				if (fillingRatio > FILL_RATIO_PERCENTAGE) {
					for (int m = 0; m < componentPixels; m++) {
						Point aux;
						aux = currentComponent[m];
						image[aux.x * w + aux.y] = 0;
					}
				}
			}

			componentPixels = 0;
			labelValue++;
		}
	}
}

void CDibView::OnProcessingConnectedcomponent()
{
	BEGIN_PROCESSING();

	int height = (clickCoordinates.x != -1 && clickCoordinates.y != -1) ? clickCoordinates.y : dwHeight;
	connectedComponentLabeling(lpDst, height, dwWidth, w, 0);

	END_PROCESSING("Connected component");
}

#define HORIZON_LINE_COLOR (0xFFFFFFFF)

void bresenhamLineDrawing(BYTE* image, Line line, int width, int height, int w) {
	int x1 = line.p1.x;
	int y1 = line.p1.y;
	int x2 = line.p2.x;
	int y2 = line.p2.y;

	const bool steep = (abs(y2 - y1) > abs(x2 - x1));
	if (steep) {
		swap(x1, y1);
		swap(x2, y2);
	}

	if (x1 > x2) {
		swap(x1, x2);
		swap(y1, y2);
	}

	const int dx = x2 - x1;
	const int dy = abs(y2 - y1);

	double error = dx / 2.0f;
	const int ystep = (y1 < y2) ? 1 : -1;
	int y = y1;

	const int maxX = x2;

	for (int x = x1; x < maxX; x++) {
		if (steep) {
			Point point = { y, x };
			if ((point.x >= 0 && point.x < height) && (point.y >= 0 && point.y < width))
				image[point.x * w + point.y] = 700;
		}
		else {
			Point point = { x, y };

			if ((point.x >= 0 && point.x < height) && (point.y >= 0 && point.y < width))
				image[point.x * w + point.y] = 700;
		}

		error -= dy;
		if (error < 0) {
			y += ystep;
			error += dx;
		}
	}
}

void bresenhamLinePointExtraction(Line line, vector<Point>* linePixels, int width, int height, int w, int horizonLineRowPosition) {
	int x1 = line.p1.x;
	int y1 = line.p1.y;
	int x2 = line.p2.x;
	int y2 = line.p2.y;

	vector<Point> pixels;

	const bool steep = (abs(y2 - y1) > abs(x2 - x1));
	if (steep) {
		swap(x1, y1);
		swap(x2, y2);
	}

	if (x1 > x2) {
		swap(x1, x2);
		swap(y1, y2);
	}

	const int dx = x2 - x1;
	const int dy = abs(y2 - y1);

	double error = dx / 2.0f;
	const int ystep = (y1 < y2) ? 1 : -1;
	int y = y1;

	const int maxX = x2;

	for (int x = x1; x < maxX; x++) {
		if (steep) {
			Point point = { x, y };

			if ((point.x >= 0 && point.x < horizonLineRowPosition) && (point.y >= 0 && point.y < width)) {
				pixels.push_back(point);
			}
		}
		else {
			Point point = { y, x };

			if ((point.x >= 0 && point.x < horizonLineRowPosition) && (point.y >= 0 && point.y < width)) {
				pixels.push_back(point);
			}
		}

		error -= dy;
		if (error < 0) {
			y += ystep;
			error += dx;
		}
	}

	*linePixels = pixels;
}

double angleBetweenLines(Line line1, Line line2)
{
	double a = (float)line1.p1.x - line1.p2.x;
	double b = (float)line1.p1.y - line1.p2.y;
	double c = (float)line2.p1.x - line2.p2.x;
	double d = (float)line2.p1.y - line2.p2.y;

	double cos_angle, angle;
	double mag_v1 = sqrt(a*a + b*b);
	double mag_v2 = sqrt(c*c + d*d);

	cos_angle = (a*c + b*d) / (mag_v1 * mag_v2);
	angle = acos(cos_angle);
	angle = angle * 180.0 / PI;

	return angle;
}

void edgeExtraction(BYTE* image, BYTE* canny, vector<Line>* vanishingLines, vector<Line>* detectionResults, int width, int height, int w, int horizonLineRowPosition) {
	int EDGE_MASK_HEIGHT = 1;
	int EDGE_MASK_WIDTH = 3;

	int MIN_LINE_THRESHOLD = 25;
	int MAX_GAP_THRESHOLD = 5;

	bool valid, maybe;

	vector<Point> pixels;
	vector<Point> validPixels;
	vector<Point> maybePixels;
	vector<Line> validLines;

	Point p1, p2;
	Line line;
	int size = vanishingLines->size();
	for (int k = 0; k < size; k++) {
		line = vanishingLines->at(k);

		bresenhamLinePointExtraction(line, &pixels, width, height, w, horizonLineRowPosition);

		valid = false;
		maybe = false;

		for (int k = 0; k < pixels.size(); k++) {
			Point pixel = pixels[k];
			//image[pixel.x * w + pixel.y] = 0;

			bool found = false;
			for (int i = (pixel.x - EDGE_MASK_HEIGHT >= 0 ? pixel.x - EDGE_MASK_HEIGHT : 0); i <= (pixel.x + EDGE_MASK_HEIGHT < horizonLineRowPosition ? pixel.x + EDGE_MASK_HEIGHT : horizonLineRowPosition - 1); i++) {
				for (int j = (pixel.y - EDGE_MASK_WIDTH >= 0 ? pixel.y - EDGE_MASK_WIDTH : 0); j <= (pixel.y + EDGE_MASK_WIDTH < width ? pixel.y + EDGE_MASK_WIDTH : width - 1); j++) {
					if (canny[pixel.x * w + pixel.y] == 255) {
						found = true;
						break;
					}
				}

				if (found)
					break;
			}

			if (found) {
				//image[pixel.x * w + pixel.y] = 700;

				valid = true;

				if (maybe) {
					if (maybePixels.size() <= MAX_GAP_THRESHOLD && maybePixels.size() > 0)
						validPixels.insert(validPixels.end(), maybePixels.begin(), maybePixels.end());
					else
						valid = false;

					maybe = false;
					maybePixels = vector<Point>();
				}

				if (valid) {
					if (pixel == pixels.back()) {
						validPixels.push_back(pixel);

						if (validPixels.size() >= MIN_LINE_THRESHOLD) {
							p1 = validPixels.front();
							p2 = validPixels.back();
							if (p1.x < p2.x)
								swap(p1, p2);
							validLines.push_back({ p1, p2 });
						}

						validPixels = vector<Point>();
						valid = false;
						maybe = false;
					}
					else
						validPixels.push_back(pixel);
				}
				else {
					if (validPixels.size() >= MIN_LINE_THRESHOLD) {
						p1 = validPixels.front();
						p2 = validPixels.back();
						if (p1.x < p2.x)
							swap(p1, p2);
						validLines.push_back({ p1, p2 });
					}
					valid = false;
					maybe = false;
					validPixels = vector<Point>();
				}
			}
			else {
				if (valid) {
					maybe = true;

					if ((maybePixels.size() > MAX_GAP_THRESHOLD || pixel == pixels.back()) && validPixels.size() >= MIN_LINE_THRESHOLD) {
						p1 = validPixels.front();
						p2 = validPixels.back();
						if (p1.x < p2.x)
							swap(p1, p2);
						validLines.push_back({ p1, p2 });

						valid = false;
						maybe = false;
						validPixels = vector<Point>();
						maybePixels = vector<Point>();
					}
					else {
						maybePixels.push_back(pixel);
					}
				}
			}
		}

		maybePixels = vector<Point>();
		validPixels = vector<Point>();
	}

	vector<Point>().swap(pixels);
	vector<Point>().swap(validPixels);
	vector<Point>().swap(maybePixels);

	int size2 = validLines.size();

	if (size2 == 0 && size2 < 5)
		return;	//No detection;

	//RANSAC parameters;
	int distance = 5;
	double p = 0.99;
	double w1 = 0.2;
	double e = 0.7;
	int n = abs(log(1 - p) / log(1 - w1*w1));

	vector<Point> finalPointsAbove;
	vector<Point> tempPointsAbove;

	vector<Point> finalPointsBelow;
	vector<Point> tempPointsBelow;

	int index1;
	int index2;

	Line lineAbove;
	Line lineBelow;

	Line line1;
	Line line2;
	for (int i = 0; i <= n; i++) {

		index1 = rand() % size2;
		index2 = rand() % size2;

		while (index1 == index2) {
			index2 = rand() % size2;
		}

		line1 = validLines[index1];
		line2 = validLines[index2];

		float a1 = line1.p1.x - line2.p1.x;
		float b1 = line2.p1.y - line1.p1.y;
		float c1 = line1.p1.y * line2.p1.x - line2.p1.y * line1.p1.x;

		float a2 = line1.p2.x - line2.p2.x;
		float b2 = line2.p2.y - line1.p2.y;
		float c2 = line1.p2.y * line2.p2.x - line2.p2.y * line1.p2.x;

		for (int i = 0; i < size2; i++) {
			float d1 = abs(a1 * validLines[i].p1.y + b1 * validLines[i].p1.x + c1) / sqrt(a1*a1 + b1*b1);
			float d2 = abs(a2 * validLines[i].p2.y + b2 * validLines[i].p2.x + c2) / sqrt(a2*a2 + b2*b2);

			if (d1 <= distance)
				tempPointsAbove.push_back(validLines[i].p1);

			if (d2 <= distance)
				tempPointsBelow.push_back(validLines[i].p2);
		}

		if (finalPointsAbove.size() < tempPointsAbove.size()) {
			finalPointsAbove = tempPointsAbove;
			tempPointsAbove = vector<Point>();

			if (line1.p1.y > line2.p1.y)
				swap(line1, line2);

			lineAbove = { line1.p1, line2.p1 };
		}
		else
			tempPointsAbove = vector<Point>();

		if (finalPointsBelow.size() < tempPointsBelow.size()) {
			finalPointsBelow = tempPointsBelow;
			tempPointsBelow = vector<Point>();

			if (line1.p2.y > line2.p2.y)
				swap(line1, line2);

			lineBelow = { line1.p2, line2.p2 };
		}
		else
			tempPointsBelow = vector<Point>();
	}

	lineIntersection(lineAbove, { { 0, 0 } ,{ height, 0 } }, &p1);
	lineIntersection(lineAbove, { { 0, width },{ height, width } }, &p2);
	swap(p1.x, p1.y);
	swap(p2.x, p2.y);

	Line finalLineAbove = { p1 , p2 };

	lineIntersection(lineBelow, { { height, 0 } ,{ 0, 0 } }, &p1);
	lineIntersection(lineBelow, { { height, width },{ 0, width } }, &p2);
	swap(p1.x, p1.y);
	swap(p2.x, p2.y);
	Line finalLineBelow = { p1 , p2 };

	vector<Line> detections;
	if (angleBetweenLines(finalLineAbove, finalLineBelow) < 2) {
		detections.push_back(finalLineAbove);
		detections.push_back(finalLineBelow);
	}

	*detectionResults = detections;

	vector<Point>().swap(finalPointsAbove);
	vector<Point>().swap(tempPointsAbove);
	vector<Point>().swap(finalPointsBelow);
	vector<Point>().swap(tempPointsBelow);

	vector<Line>().swap(validLines);
	vector<Line>().swap(detections);

}

void CDibView::OnProcessingHough()
{
	BEGIN_PROCESSING();

	CDC dc;
	dc.CreateCompatibleDC(0);
	CBitmap ddBitmap;
	HBITMAP hDDBitmap = CreateDIBitmap(::GetDC(0), &((LPBITMAPINFO)lpS)->bmiHeader, CBM_INIT, lpSrc, (LPBITMAPINFO)lpS, DIB_RGB_COLORS);
	ddBitmap.Attach(hDDBitmap);
	CBitmap* pTempBmp = dc.SelectObject(&ddBitmap);
	CPen pen_red(PS_SOLID, 1, RGB(255, 0, 0));
	CPen *pTempPen = dc.SelectObject(&pen_red);

	int height = (clickCoordinates.x != -1 && clickCoordinates.y != -1) ? clickCoordinates.y : dwHeight;
	vector<Line>lines;
	houghLineTransform(lpSrc, &lines, height, dwWidth, w);

	Line line;
	for (int i = 0; i < lines.size(); i++) {
		line = lines[i];

		dc.MoveTo(line.p1.x, dwHeight - line.p1.y - 1);
		dc.LineTo(line.p2.x, dwHeight - line.p2.y - 1);
	}

	vector<Line>().swap(lines);

	dc.SelectObject(pTempPen);
	dc.SelectObject(pTempBmp);
	GetDIBits(dc.m_hDC, ddBitmap, 0, dwHeight, lpDst, (LPBITMAPINFO)lpD, DIB_RGB_COLORS);

	pen_red.DeleteObject();
	pTempPen->DeleteObject();

	END_PROCESSING("Hough");
}

void CDibView::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	BEGIN_SOURCE_PROCESSING;

	//obtain the scroll position (because of scroll bars' positions
	//the coordinates may be shifted) and adjust the position
	CPoint pos = GetScrollPosition() + point;
	//point contains the window's client area coordinates
	//the y axis is inverted because of the way bitmaps
	//are represented in memory

	int x = pos.x;
	int y = dwHeight - pos.y - 1;

	//test if the position is inside the image
	if (x > 0 && x < dwWidth && y>0 && y < dwHeight)
	{
		clickCoordinates.x = x;
		clickCoordinates.y = y;

		//prepare a CString for formating the output message
		if (debugMode) {
			CString info;
			info.Format(_TEXT("Point: x=%d, y=%d, color=%d"), x, y, lpSrc[y*w + x]);
			AfxMessageBox(info);
		}
	}
	END_SOURCE_PROCESSING;

	//call the superclass' method
	CScrollView::OnLButtonDblClk(nFlags, point);
}

void CDibView::OnRButtonDown(UINT nFlags, CPoint point)
{
	BEGIN_SOURCE_PROCESSING;
	clickCoordinates.x = -1;
	clickCoordinates.y = -1;
	END_SOURCE_PROCESSING;

	//call the superclass' method
	CScrollView::OnRButtonDown(nFlags, point);
}


void CDibView::OnAlgorithmDetect()
{
	BEGIN_PROCESSING();
	CDC dc;
	dc.CreateCompatibleDC(0);
	CBitmap ddBitmap;
	HBITMAP hDDBitmap = CreateDIBitmap(::GetDC(0), &((LPBITMAPINFO)lpS)->bmiHeader, CBM_INIT, lpSrc, (LPBITMAPINFO)lpS, DIB_RGB_COLORS);
	ddBitmap.Attach(hDDBitmap);
	CBitmap* pTempBmp = dc.SelectObject(&ddBitmap);
	CPen pen_red(PS_SOLID, 1, RGB(255, 0, 0));
	CPen pen_green(PS_SOLID, 1, RGB(0, 255, 0));
	CPen *pTempPen = dc.SelectObject(&pen_red);

	int height = dwHeight;
	int width = dwWidth;
	int heightHorizon = (clickCoordinates.x != -1 && clickCoordinates.y != -1) ? clickCoordinates.y : height;

	convert2Grayscale(lpSrc, bmiColorsSrc, iColors, w, dwHeight, dwWidth);

	BYTE* grayscale = new BYTE[height * width];
	memcpy(grayscale, lpSrc, height*width);

	if (!computedLookupTables)
		computeLookUpTables();

	BYTE* canny = new BYTE[height * width];
	memcpy(canny, grayscale, height*width);
	cannyEdgeDetection(canny, heightHorizon, width, w);

	BYTE* cannyCopy = new BYTE[height * width];
	memcpy(cannyCopy, canny, height*width);

	morphologicalDilation(canny, heightHorizon, width);

	morphologicalErosion(canny, heightHorizon, width);

	connectedComponentLabeling(canny, heightHorizon, width);

	for (int i = 0; i < height; i++)
		for (int j = 0; j < width; j++)
			if (cannyCopy[i*w + j] == 0)
				canny[i*w + j] = 0;

	vector<Line>lines;
	houghLineTransform(canny, &lines, heightHorizon, width, w);
	Line line1;
	//for (int i = 0; i < lines.size(); i++) {
	//	line1 = lines[i];
	//	dc.MoveTo(line1.p1.x, dwHeight - line1.p1.y - 1);
	//	dc.LineTo(line1.p2.x, dwHeight - line1.p2.y - 1);
	//}

	pTempPen = dc.SelectObject(&pen_green);

	Point vanishingPoint;
	vector<Line> vanishingLines;
	vanishingLinesExtractionRansac(&lines, &vanishingLines, &vanishingPoint, width, height);
	/*for (int i = 0; i < vanishingLines.size(); i++) {
		line1 = vanishingLines[i];

		dc.MoveTo(line1.p1.x, dwHeight - line1.p1.y - 1);
		dc.LineTo(line1.p2.x, dwHeight - line1.p2.y - 1);
	}
	dc.Ellipse(vanishingPoint.x - 5, height - vanishingPoint.y - 1 - 5, vanishingPoint.x + 5, height - vanishingPoint.y - 1 + 5);*/

	vector<Line> detectionResults;
	edgeExtraction(lpDst, canny, &vanishingLines, &detectionResults, width, height, w, heightHorizon);
	for (int i = 0; i < detectionResults.size(); i++) {
		line1 = detectionResults[i];

		dc.MoveTo(line1.p1.x, dwHeight - line1.p1.y - 1);
		dc.LineTo(line1.p2.x, dwHeight - line1.p2.y - 1);
	}

	vector<Line>().swap(lines);
	vector<Line>().swap(vanishingLines);
	vector<Line>().swap(detectionResults);

	dc.SelectObject(pTempPen);
	dc.SelectObject(pTempBmp);
	GetDIBits(dc.m_hDC, ddBitmap, 0, dwHeight, lpDst, (LPBITMAPINFO)lpD, DIB_RGB_COLORS);

	pen_red.DeleteObject();
	pTempPen->DeleteObject();
	
	delete[] grayscale;
	delete[] canny;
	delete[] cannyCopy;

	END_PROCESSING("Detection");
}
