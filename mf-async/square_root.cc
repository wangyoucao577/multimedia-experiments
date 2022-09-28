
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

  IMFAsyncResult *pResult = NULL;

  // Create the inner result object. This object contains pointers to:
  //
  //   1. The caller's callback interface and state object.
  //   2. The AsyncOp object, which contains the operation data.
  //
  HRESULT hr = MFCreateAsyncResult(pOp, pCB, pState, &pResult);

  if (SUCCEEDED(hr)) {
    // Queue a work item. The work item contains pointers to:
    //
    // 1. The callback interface of the SuqareRoot object.
    // 2. The inner result object.
    hr = MFPutWorkItem(MFASYNC_CALLBACK_QUEUE_STANDARD, this, pResult);

    pResult->Release();
  }

  return hr;
}

// Invoke is called by the work queue. This is where the object performs the
// asynchronous operation.

STDMETHODIMP SquareRoot::Invoke(IMFAsyncResult *pResult) {
  HRESULT hr = S_OK;

  IUnknown *pState = NULL;
  IUnknown *pUnk = NULL;
  IMFAsyncResult *pCallerResult = NULL;

  AsyncOp *pOp = NULL;

  // Get the asynchronous result object for the application callback.

  hr = pResult->GetState(&pState);
  if (FAILED(hr)) {
    goto done;
  }

  hr = pState->QueryInterface(IID_PPV_ARGS(&pCallerResult));
  if (FAILED(hr)) {
    goto done;
  }

  // Get the object that holds the state information for the asynchronous
  // method.
  hr = pCallerResult->GetObject(&pUnk);
  if (FAILED(hr)) {
    goto done;
  }

  pOp = static_cast<AsyncOp *>(pUnk);

  // Do the work.

  hr = DoCalculateSquareRoot(pOp);

done:
  // Signal the application.
  if (pCallerResult) {
    pCallerResult->SetStatus(hr);
    MFInvokeCallback(pCallerResult);
  }

  SafeRelease(&pState);
  SafeRelease(&pUnk);
  SafeRelease(&pCallerResult);
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
