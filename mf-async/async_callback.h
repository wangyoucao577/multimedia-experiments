#include <mfapi.h>
#include <shlwapi.h>

class AsyncCallback : public IMFAsyncCallback {
public:
  AsyncCallback() : m_cRef(1) {}
  virtual ~AsyncCallback() {}

  STDMETHODIMP QueryInterface(REFIID riid, void **ppv) override {
    static const QITAB qit[] = {QITABENT(AsyncCallback, IMFAsyncCallback), {0}};
    return QISearch(this, qit, riid, ppv);
  }

  STDMETHODIMP_(ULONG) AddRef() override {
    return InterlockedIncrement(&m_cRef);
  }
  STDMETHODIMP_(ULONG) Release() override {
    long cRef = InterlockedDecrement(&m_cRef);
    if (cRef == 0) {
      delete this;
    }
    return cRef;
  }

  STDMETHODIMP GetParameters(DWORD *pdwFlags, DWORD *pdwQueue) override {
    // Implementation of this method is optional.
    return E_NOTIMPL;
  }

  STDMETHODIMP Invoke(IMFAsyncResult *pAsyncResult) override = 0;
  // TODO: Implement this method.

  // Inside Invoke, IMFAsyncResult::GetStatus to get the status.
  // Then call the EndX method to complete the operation.

private:
  long m_cRef;
};
