#include "io/solidworks/SolidWorksConverter.h"
#include <comutil.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <windows.h>

// Link against comsuppw.lib (usually standard, but explicit for MinGW/MSVC
// might be needed) MinGW might need -lole32 -loleaut32 -luuid

namespace opencad {
namespace io {

SolidWorksConverter::SolidWorksConverter() {}
SolidWorksConverter::~SolidWorksConverter() {}

std::string SolidWorksConverter::errorMessage() const { return m_error; }

// Helper for BSTR allocation
BSTR ConvertStringToBSTR(const std::string &str) {
  int wslen = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, 0, 0);
  BSTR bstr = SysAllocStringLen(0, wslen);
  MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, bstr, wslen);
  return bstr;
}

bool SolidWorksConverter::convertToStep(const std::string &inputFile,
                                        std::string &resultingStepFile) {
  std::ofstream log("sw_export_log.txt");
  log << "Starting conversion for: " << inputFile << "\n";

  HRESULT hr = CoInitialize(NULL);
  if (FAILED(hr)) {
    m_error = "Failed to initialize COM library.";
    log << "Error: " << m_error << " (HR=" << std::hex << hr << ")\n";
    return false;
  }

  // Prepare temp output path
  std::filesystem::path inputPath(inputFile);
  std::filesystem::path outputPath =
      std::filesystem::temp_directory_path() / "opencad_sw_export.step";
  resultingStepFile = outputPath.string();
  log << "Output file: " << resultingStepFile << "\n";

  // Create SldWorks Application Instance
  CLSID clsid;
  hr = CLSIDFromProgID(L"SldWorks.Application", &clsid);
  if (FAILED(hr)) {
    m_error = "SolidWorks is not installed or registered (ProgID not found).";
    log << "Error: " << m_error << " (HR=" << std::hex << hr << ")\n";
    CoUninitialize();
    return false;
  }

  IDispatch *pSldWorks = NULL;
  hr = CoCreateInstance(clsid, NULL, CLSCTX_LOCAL_SERVER, IID_IDispatch,
                        (void **)&pSldWorks);
  if (FAILED(hr) || !pSldWorks) {
    m_error = "Failed to launch SolidWorks (CoCreateInstance failed).";
    log << "Error: " << m_error << " (HR=" << std::hex << hr << ")\n";
    CoUninitialize();
    return false;
  }
  log << "SolidWorks launched successfully.\n";

  // Make it visible (Optional, usually better hidden but visible for debugging)
  {
    DISPID dispidVisible = 0;
    OLECHAR *nameVisible = (OLECHAR *)L"Visible";
    hr = pSldWorks->GetIDsOfNames(IID_NULL, &nameVisible, 1,
                                  LOCALE_USER_DEFAULT, &dispidVisible);
    if (SUCCEEDED(hr)) {
      VARIANTARG args[1];
      args[0].vt = VT_BOOL;
      args[0].boolVal = VARIANT_TRUE;
      DISPPARAMS params = {args, NULL, 1, 0};
      pSldWorks->Invoke(dispidVisible, IID_NULL, LOCALE_USER_DEFAULT,
                        DISPATCH_PROPERTYPUT, &params, NULL, NULL, NULL);
      log << "SolidWorks made visible.\n";
    } else {
      log << "Warning: Could not set Visible property (HR=" << std::hex << hr
          << ")\n";
    }
  }

  // OpenDoc6(FileName, Type, Options, Configuration, Errors, Warnings)
  // DISPID "OpenDoc6"
  DISPID dispidOpen = 0;
  OLECHAR *nameOpen = (OLECHAR *)L"OpenDoc6";
  hr = pSldWorks->GetIDsOfNames(IID_NULL, &nameOpen, 1, LOCALE_USER_DEFAULT,
                                &dispidOpen);
  if (FAILED(hr)) {
    m_error = "Failed to find OpenDoc6 method.";
    log << "Error: " << m_error << " (HR=" << std::hex << hr << ")\n";
    pSldWorks->Release();
    CoUninitialize();
    return false;
  }

  VARIANTARG args[6];
  // Args are passed in REVERSE order for Dispatch Invoke!

  // Args[0] must be the LAST argument of the function signature (Warnings)
  VariantInit(&args[0]);
  args[0].vt = VT_BYREF | VT_I4;
  long warnings = 0;
  args[0].plVal = &warnings;

  // Args[1] must be the 2nd to LAST argument (Errors)
  VariantInit(&args[1]);
  args[1].vt = VT_BYREF | VT_I4;
  long errors = 0;
  args[1].plVal = &errors;

  // Args[2] -> Config
  VariantInit(&args[2]);
  args[2].vt = VT_BSTR;
  args[2].bstrVal = SysAllocString(L"");

  // Args[3] -> Options
  VariantInit(&args[3]);
  args[3].vt = VT_I4;
  args[3].lVal = 1; // Silent

  // Args[4] -> Type
  VariantInit(&args[4]);
  std::string ext = inputPath.extension().string();
  if (ext == ".SLDASM" || ext == ".sldasm")
    args[4].lVal = 2;
  else
    args[4].lVal = 1;
  args[4].vt = VT_I4;

  // Args[5] -> FileName (First argument of signature)
  VariantInit(&args[5]);
  std::wstring wPath = std::filesystem::absolute(inputPath).wstring();
  args[5].vt = VT_BSTR;
  args[5].bstrVal = SysAllocString(wPath.c_str());

  log << "Opening file path: " << std::filesystem::absolute(inputPath).string()
      << "\n";

  DISPPARAMS params = {args, NULL, 6, 0};
  VARIANT result;
  VariantInit(&result);

  hr = pSldWorks->Invoke(dispidOpen, IID_NULL, LOCALE_USER_DEFAULT,
                         DISPATCH_METHOD, &params, &result, NULL, NULL);

  log << "OpenDoc6 Invoke result: " << std::hex << hr << "\n";
  log << "Open Errors: " << errors << ", Warnings: " << warnings << "\n";

  IDispatch *pModelDoc = NULL;
  // Check if result is indeed a dispatch
  if (SUCCEEDED(hr) && result.vt == VT_DISPATCH) {
    pModelDoc = result.pdispVal;
  }

  // Cleanup input args
  SysFreeString(args[5].bstrVal); // FileName
  SysFreeString(args[2].bstrVal); // Config

  if (!pModelDoc) {
    m_error = "Failed to open document. Error code: " + std::to_string(errors);
    log << "Error: " << m_error << "\n";
    pSldWorks->Release();
    CoUninitialize();
    return false;
  }
  log << "Document opened successfully.\n";

  // SaveAs3(Name, Version, Options, Data, Options2, Errors, Warnings)
  // We can use SaveAs(Name) simpler?
  // Let's use SaveAs3 for robustness if possible, but SaveAs is easier to
  // invoke. Try "SaveAs" first.
  // Use explicit SaveAs3 to avoid BadParamCount
  // Signature: SaveAs3(Name, Version, Options, Data, Options2, Errors,
  // Warnings) Reversed for Invoke: 0: Warnings 1: Errors 2: Options2 3: Data 4:
  // Options 5: Version 6: Name

  // Robust SaveAs approach: Try multiple signatures until one works.

  std::wstring wOutPath = outputPath.wstring();

  // UNROLLING FOR SAFETY

  // ATTEMPT 1: SaveAs(Name)
  if (!std::filesystem::exists(outputPath)) {
    DISPID dispid = 0;
    OLECHAR *name = (OLECHAR *)L"SaveAs";
    if (SUCCEEDED(pModelDoc->GetIDsOfNames(IID_NULL, &name, 1,
                                           LOCALE_USER_DEFAULT, &dispid))) {
      VARIANTARG args[1];
      VariantInit(&args[0]);
      args[0].vt = VT_BSTR;
      args[0].bstrVal = SysAllocString(wOutPath.c_str());
      DISPPARAMS params = {args, NULL, 1, 0};
      VARIANT res;
      VariantInit(&res);
      hr = pModelDoc->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT,
                             DISPATCH_METHOD, &params, &res, NULL, NULL);
      log << "SaveAs(1) result: " << std::hex << hr << "\n";
      SysFreeString(args[0].bstrVal);
    }
  }

  // ATTEMPT 2: SaveAs(Name, Version, Options)
  // Reversed: [0] Options, [1] Version, [2] Name
  if (!std::filesystem::exists(outputPath)) {
    DISPID dispid = 0;
    OLECHAR *name = (OLECHAR *)L"SaveAs";
    if (SUCCEEDED(pModelDoc->GetIDsOfNames(IID_NULL, &name, 1,
                                           LOCALE_USER_DEFAULT, &dispid))) {
      VARIANTARG args[3];
      VariantInit(&args[0]);
      args[0].vt = VT_I4;
      args[0].lVal = 0; // Options
      VariantInit(&args[1]);
      args[1].vt = VT_I4;
      args[1].lVal = 0; // Version
      VariantInit(&args[2]);
      args[2].vt = VT_BSTR;
      args[2].bstrVal = SysAllocString(wOutPath.c_str()); // Name

      DISPPARAMS params = {args, NULL, 3, 0};
      VARIANT res;
      VariantInit(&res);
      hr = pModelDoc->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT,
                             DISPATCH_METHOD, &params, &res, NULL, NULL);
      log << "SaveAs(3) result: " << std::hex << hr << "\n";
      SysFreeString(args[2].bstrVal);
    }
  }

  // ATTEMPT 3: SaveAs3(Name, Version, Options)
  // Reversed: [0] Options, [1] Version, [2] Name
  if (!std::filesystem::exists(outputPath)) {
    DISPID dispid = 0;
    OLECHAR *name = (OLECHAR *)L"SaveAs3";
    if (SUCCEEDED(pModelDoc->GetIDsOfNames(IID_NULL, &name, 1,
                                           LOCALE_USER_DEFAULT, &dispid))) {
      VARIANTARG args[3];
      VariantInit(&args[0]);
      args[0].vt = VT_I4;
      args[0].lVal = 0;
      VariantInit(&args[1]);
      args[1].vt = VT_I4;
      args[1].lVal = 0;
      VariantInit(&args[2]);
      args[2].vt = VT_BSTR;
      args[2].bstrVal = SysAllocString(wOutPath.c_str());

      DISPPARAMS params = {args, NULL, 3, 0};
      VARIANT res;
      VariantInit(&res);
      hr = pModelDoc->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT,
                             DISPATCH_METHOD, &params, &res, NULL, NULL);
      log << "SaveAs3(3) result: " << std::hex << hr << "\n";
      SysFreeString(args[2].bstrVal);
    }
  }

  // CloseDoc
  DISPID dispidQuit = 0;
  OLECHAR *nameQuit = (OLECHAR *)L"QuitDoc";
  // "CloseDoc" is safer
  OLECHAR *nameClose = (OLECHAR *)L"CloseDoc";
  hr = pSldWorks->GetIDsOfNames(IID_NULL, &nameClose, 1, LOCALE_USER_DEFAULT,
                                &dispidQuit);
  if (SUCCEEDED(hr)) {
    VARIANTARG closeArgs[1];
    closeArgs[0].vt = VT_BSTR;
    closeArgs[0].bstrVal = SysAllocString(wPath.c_str());
    DISPPARAMS closeParams = {closeArgs, NULL, 1, 0};
    pSldWorks->Invoke(dispidQuit, IID_NULL, LOCALE_USER_DEFAULT,
                      DISPATCH_METHOD, &closeParams, NULL, NULL, NULL);
    SysFreeString(closeArgs[0].bstrVal);
  }

  pModelDoc->Release();
  pSldWorks->Release();
  CoUninitialize();

  if (std::filesystem::exists(outputPath)) {
    log << "Export confirmed: File exists.\n";
    return true;
  } else {
    m_error = "File was not saved to output path.";
    log << "Error: File not found at output path. All SaveAs attempts "
           "failed.\n";
    return false;
  }
}

} // namespace io
} // namespace opencad
