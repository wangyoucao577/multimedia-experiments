
#include <mfapi.h>
#include <shlwapi.h>

class AsyncOp : public IUnknown {
  LONG m_cRef;

public:
  double m_value;

  AsyncOp(double val) : m_cRef(1), m_value(val) {}

  STDMETHODIMP QueryInterface(REFIID riid, void **ppv) {
    static const QITAB qit[] = {QITABENT(AsyncOp, IUnknown), {0}};
    return QISearch(this, qit, riid, ppv);
  }

  STDMETHODIMP_(ULONG) AddRef() { return InterlockedIncrement(&m_cRef); }

  STDMETHODIMP_(ULONG) Release() {
    LONG cRef = InterlockedDecrement(&m_cRef);
    if (cRef == 0) {
      delete this;
    }
    return cRef;
  }
};

class SquareRoot : public IMFAsyncCallback {
  LONG m_cRef;

  HRESULT DoCalculateSquareRoot(AsyncOp *pOp);

public:
  SquareRoot() : m_cRef(1) {}

  // Async APIs to dispatch real calculation to work queue
  HRESULT BeginSquareRoot(double x, IMFAsyncCallback *pCB, IUnknown *pState);
  HRESULT EndSquareRoot(IMFAsyncResult *pResult, double *pVal);

  // IUnknown methods.

  STDMETHODIMP QueryInterface(REFIID riid, void **ppv) override {
    static const QITAB qit[] = {QITABENT(SquareRoot, IMFAsyncCallback), {0}};
    return QISearch(this, qit, riid, ppv);
  }

  STDMETHODIMP_(ULONG) AddRef() override {
    return InterlockedIncrement(&m_cRef);
  }

  STDMETHODIMP_(ULONG) Release() override {
    LONG cRef = InterlockedDecrement(&m_cRef);
    if (cRef == 0) {
      delete this;
    }
    return cRef;
  }

  // IMFAsyncCallback methods.

  STDMETHODIMP GetParameters(DWORD *pdwFlags, DWORD *pdwQueue) override {
    // Implementation of this method is optional.
    return E_NOTIMPL;
  }
  // Invoke is where the work is performed.
  STDMETHODIMP Invoke(IMFAsyncResult *pResult) override;
};
