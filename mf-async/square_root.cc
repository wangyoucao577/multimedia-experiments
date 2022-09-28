
#include "square_root.h"
#include <new>

template <class T> void SafeRelease(T **ppT) {
  if (*ppT) {
    (*ppT)->Release();
    *ppT = NULL;
  }
}

HRESULT SquareRoot::BeginSquareRoot(double x, IMFAsyncCallback *pCB,
                                    IUnknown *pState) {
  AsyncOp *pOp = new (std::nothrow) AsyncOp(x);
  if (pOp == NULL) {
    return E_OUTOFMEMORY;
  }

  IMFAsyncResult *pResult1 = NULL;

  // Create the inner result object. This object contains pointers to:
  //
  //   1. The caller's callback interface and state object.
  //   2. The AsyncOp object, which contains the operation data.
  //
  HRESULT hr = MFCreateAsyncResult(pOp, pCB, pState, &pResult1);

  if (SUCCEEDED(hr)) {

    // Queue a work item. The work item contains pointers to:
    //
    // 1. The callback interface of the SuqareRoot object.
    // 2. The inner result object.
    {
      IMFAsyncResult *pResult2 = NULL;
      hr = MFCreateAsyncResult(nullptr, this, pResult1, &pResult2);
      if (SUCCEEDED(hr)) {
        hr = MFPutWorkItemEx(MFASYNC_CALLBACK_QUEUE_STANDARD, pResult2);
        pResult2->Release();
      }

      // Alternatively, same thing can be achieved by `MFPutWorkItem`
      // hr = MFPutWorkItem(MFASYNC_CALLBACK_QUEUE_STANDARD, this, pResult1);
    }

    pResult1->Release();
  }

  return hr;
}

// Invoke is called by the work queue. This is where the object performs the
// asynchronous operation.

STDMETHODIMP SquareRoot::Invoke(IMFAsyncResult *pResult2) {
  HRESULT hr = S_OK;

  IUnknown *pState = NULL; // pResult1
  IUnknown *pUnk = NULL;
  IMFAsyncResult *pResult1 = NULL;

  AsyncOp *pOp = NULL;

  // Get the asynchronous result object for the application callback.

  hr = pResult2->GetState(&pState);
  if (FAILED(hr)) {
    goto done;
  }

  hr = pState->QueryInterface(IID_PPV_ARGS(&pResult1));
  if (FAILED(hr)) {
    goto done;
  }

  // Get the object that holds the state information for the asynchronous
  // method.
  hr = pResult1->GetObject(&pUnk);
  if (FAILED(hr)) {
    goto done;
  }

  pOp = static_cast<AsyncOp *>(pUnk);

  // Do the work.

  hr = DoCalculateSquareRoot(pOp);

done:
  // Signal the application.
  if (pResult1) {
    pResult1->SetStatus(hr);
    MFInvokeCallback(pResult1); // call the caller's `Invoke`
  }

  SafeRelease(&pState);
  SafeRelease(&pUnk);
  SafeRelease(&pResult1);
  return S_OK;
}

HRESULT SquareRoot::DoCalculateSquareRoot(AsyncOp *pOp) {
  pOp->m_value = sqrt(pOp->m_value);

  return S_OK;
}

HRESULT SquareRoot::EndSquareRoot(IMFAsyncResult *pResult, double *pVal) {
  *pVal = 0;

  IUnknown *pUnk = NULL;

  HRESULT hr = pResult->GetStatus();
  if (FAILED(hr)) {
    goto done;
  }

  hr = pResult->GetObject(&pUnk);
  if (FAILED(hr)) {
    goto done;
  }

  AsyncOp *pOp = static_cast<AsyncOp *>(pUnk);

  // Get the result.
  *pVal = pOp->m_value;

done:
  SafeRelease(&pUnk);
  return hr;
}
