#include <cstdio>

#include <comdef.h>
#include <d2d1_1.h>
#include <wincodec.h>

#include <d2d1_3.h>
#include <shlwapi.h>

#define COM_SMARTPTR_TYPEDEF(a) _COM_SMARTPTR_TYPEDEF(a, __uuidof(a))
#define CHK(hr) check_hresult(hr, __FILE__, __LINE__)

inline void check_hresult(HRESULT hr, const char *file, int line) {
    if (!SUCCEEDED(hr)) {
        std::fprintf(stdout, "%s:%d: bad HRESULT 0x%08x\n", file, line, (unsigned)hr);
        std::fflush(stdout);
        throw _com_error(hr);
    }
}

COM_SMARTPTR_TYPEDEF(IWICImagingFactory);
COM_SMARTPTR_TYPEDEF(IWICBitmap);
COM_SMARTPTR_TYPEDEF(IWICStream);
COM_SMARTPTR_TYPEDEF(IWICBitmapEncoder);
COM_SMARTPTR_TYPEDEF(IWICBitmapFrameEncode);
COM_SMARTPTR_TYPEDEF(ID2D1Brush);
COM_SMARTPTR_TYPEDEF(ID2D1RenderTarget);
COM_SMARTPTR_TYPEDEF(ID2D1Factory1);
COM_SMARTPTR_TYPEDEF(ID2D1SolidColorBrush);

COM_SMARTPTR_TYPEDEF(IStream);
COM_SMARTPTR_TYPEDEF(ID2D1DeviceContext5);
COM_SMARTPTR_TYPEDEF(ID2D1SvgDocument);

void convert(const wchar_t* svg_filename, const wchar_t *output_filename) {

    static const UINT Width = 400;
    static const UINT Height = 400;

    // Create a WIC (Windows Imaging Component) factory.
    IWICImagingFactoryPtr WICImagingFactory;
    CHK(WICImagingFactory.CreateInstance(
        CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER));

    // Create a WIC bitmap.
    IWICBitmapPtr WICBitmap;
    CHK(WICImagingFactory->CreateBitmap(
        Width, Height, GUID_WICPixelFormat32bppPBGRA, WICBitmapCacheOnDemand, &WICBitmap));

    // Create a Direct2D factory.
    ID2D1Factory1Ptr D2D1Factory;
    D2D1_FACTORY_OPTIONS options = {D2D1_DEBUG_LEVEL_INFORMATION};
    CHK(D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED,
        __uuidof(ID2D1Factory1), &options, (void **)&D2D1Factory));

    // Create a Direct2D render target for rendering to the WIC bitmap.
    ID2D1RenderTargetPtr D2D1RenderTarget;
    D2D1_RENDER_TARGET_PROPERTIES props =
        D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT,
            D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED), 0,
            0, D2D1_RENDER_TARGET_USAGE_NONE, D2D1_FEATURE_LEVEL_DEFAULT);
    CHK(D2D1Factory->CreateWicBitmapRenderTarget(
        WICBitmap, &props, &D2D1RenderTarget));

    // Get an ID2D1Context5.
    ID2D1DeviceContext5Ptr D2D1Context5;
    CHK(D2D1RenderTarget->QueryInterface(&D2D1Context5));

    // Open SVG file and create stream.
    IStreamPtr Stream;
    CHK(SHCreateStreamOnFileW(
        svg_filename, STGM_READ | STGM_SHARE_DENY_WRITE, &Stream));

    // Load the SVG document from the stream.
    ID2D1SvgDocumentPtr SvgDocument;
    CHK(D2D1Context5->CreateSvgDocument(Stream, { Width, Height }, &SvgDocument));

    // Draw the SVG image.
    D2D1RenderTarget->BeginDraw();
    D2D1_COLOR_F transparent{ 0.0f, 0.0f, 0.0f, 0.0f };
    D2D1RenderTarget->Clear(transparent);
    D2D1Context5->DrawSvgDocument(SvgDocument);
    CHK(D2D1RenderTarget->EndDraw());

    // Create a WIC stream.
    IWICStreamPtr WICStream;
    CHK(WICImagingFactory->CreateStream(&WICStream));
    CHK(WICStream->InitializeFromFilename(output_filename, GENERIC_WRITE));

    // Create a WIC bitmap encoder.
    IWICBitmapEncoderPtr WICEncoder;
    CHK(WICImagingFactory->CreateEncoder(
        GUID_ContainerFormatPng, nullptr, &WICEncoder));
    CHK(WICEncoder->Initialize(WICStream, WICBitmapEncoderNoCache));

    // Create a ... frame thing.
    IWICBitmapFrameEncodePtr WICBitmapFrameEncode;
    CHK(WICEncoder->CreateNewFrame(&WICBitmapFrameEncode, nullptr));

    // Save the WIC bitmap to PNG.
    CHK(WICBitmapFrameEncode->Initialize(nullptr));
    CHK(WICBitmapFrameEncode->WriteSource(WICBitmap, nullptr));
    CHK(WICBitmapFrameEncode->Commit());
    CHK(WICEncoder->Commit());
}

int wmain(int argc, const wchar_t *argv[]) {

    if (argc < 3) {
        std::fprintf(stderr, "Usage: %ls input.svg output.png", argv[0]);
        return 1;
    }

    const wchar_t* svg_filename = argv[1];
    const wchar_t *output_filename = argv[2];

    try {
        CHK(CoInitializeEx(nullptr, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE));
        convert(svg_filename, output_filename);
    } catch (...) {
        return 1;
    }
    CoUninitialize();
    return 0;
}
