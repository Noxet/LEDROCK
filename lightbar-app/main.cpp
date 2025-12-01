#include <Windows.h>
#include <dxgi1_2.h>
#include <d3d11.h>
#include <wrl.h>
#include <iostream>
#include <cstdint>

using Microsoft::WRL::ComPtr;

HANDLE serialOpen(const std::string &port, DWORD baud = CBR_115200)
{
	HANDLE handle = CreateFileA(
	    port.c_str(),
	    GENERIC_READ | GENERIC_WRITE,
	    0,
	    nullptr,
	    OPEN_EXISTING,
	    FILE_ATTRIBUTE_NORMAL,
	    nullptr
	    );

	if (handle == INVALID_HANDLE_VALUE)
	{
		std::cerr << "Failed to open " << port << "\n";
		return nullptr;
	}

	DCB dcb = { 0 };
	dcb.DCBlength = sizeof(dcb);

	if (!GetCommState(handle, &dcb)) {
		std::cerr << "GetCommState failed\n";
		CloseHandle(handle);
		return nullptr;
	}

	dcb.BaudRate = baud;
	dcb.ByteSize = 8;
	dcb.StopBits = ONESTOPBIT;
	dcb.Parity   = NOPARITY;
	dcb.fDtrControl = DTR_CONTROL_ENABLE;

	if (!SetCommState(handle, &dcb)) {
		std::cerr << "SetCommState failed\n";
		CloseHandle(handle);
		return nullptr;
	}

	// Timeout settings (non-blocking)
	/*
	COMMTIMEOUTS timeouts = { 0 };
	timeouts.ReadIntervalTimeout = 50;
	timeouts.ReadTotalTimeoutConstant = 50;
	timeouts.ReadTotalTimeoutMultiplier = 10;
	timeouts.WriteTotalTimeoutConstant = 10;
	 */

	//SetCommTimeouts(handle, &timeouts);

	return handle;
}

bool serialWrite(HANDLE handle, const uint8_t *data, int len)
{
	DWORD written;
	uint8_t msg[5];
	msg[0] = 0x04;
	msg[1] = 0x01; // static color
	memcpy(&msg[2], data, 3);
	// clean any leftover data in the software buffer. The latest data is more important
	PurgeComm(handle, PURGE_TXCLEAR);
	FlushFileBuffers(handle);
	if (!WriteFile(handle, msg, sizeof(msg), &written, nullptr)) return false;
	return written == sizeof(msg);
}

int main()
{
	HANDLE serial = serialOpen(R"(\\.\COM10)");
	if (!serial)
	{
		std::cerr << "Failed to open serial port\n";
		return EXIT_FAILURE;
	}

	HRESULT hr;

	// Create D3D11 device
	ComPtr<ID3D11Device> device;
	ComPtr<ID3D11DeviceContext> ctx;

	hr = D3D11CreateDevice(
	    nullptr,
	    D3D_DRIVER_TYPE_HARDWARE,
	    nullptr,
	    D3D11_CREATE_DEVICE_BGRA_SUPPORT,
	    nullptr, 0,
	    D3D11_SDK_VERSION,
	    &device,
	    nullptr,
	    &ctx);

	if (FAILED(hr)) {
		std::cout << "D3D11CreateDevice failed\n";
		return -1;
	}

	ComPtr<IDXGIDevice> dxgiDevice;
	device.As(&dxgiDevice);

	// Get primary output
	ComPtr<IDXGIAdapter> adapter;
	dxgiDevice->GetAdapter(&adapter);

	ComPtr<IDXGIOutput> output;
	adapter->EnumOutputs(0, &output);

	ComPtr<IDXGIOutput1> output1;
	output.As(&output1);

	// Create duplication
	ComPtr<IDXGIOutputDuplication> dup;
	hr = output1->DuplicateOutput(device.Get(), &dup);
	if (FAILED(hr)) {
		std::cout << "DuplicateOutput failed\n";
		return -2;
	}

	DXGI_OUTDUPL_DESC desc;
	dup->GetDesc(&desc);

	UINT W = desc.ModeDesc.Width;
	UINT H = desc.ModeDesc.Height;

	// We only want bottom N rows:
	const UINT NUM_ROWS = 300;

	// Create staging texture for bottom rows only
	D3D11_TEXTURE2D_DESC stagingDesc = {};
	stagingDesc.Width = W;
	stagingDesc.Height = NUM_ROWS;
	stagingDesc.MipLevels = 1;
	stagingDesc.ArraySize = 1;
	stagingDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	stagingDesc.SampleDesc.Count = 1;
	stagingDesc.Usage = D3D11_USAGE_STAGING;
	stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

	ComPtr<ID3D11Texture2D> staging;
	device->CreateTexture2D(&stagingDesc, nullptr, &staging);

	// Capture loop
	while (true)
	{
		DXGI_OUTDUPL_FRAME_INFO info = {};
		ComPtr<IDXGIResource> frameRes;

		hr = dup->AcquireNextFrame(5, &info, &frameRes);
		if (hr == DXGI_ERROR_WAIT_TIMEOUT)
			continue;
		if (FAILED(hr))
			break;

		ComPtr<ID3D11Texture2D> frameTex;
		frameRes.As(&frameTex);

		// Copy only bottom rows into staging tex
		D3D11_BOX srcBox = {};
		srcBox.left = 0;
		srcBox.right = W;
		srcBox.top = H - NUM_ROWS;
		srcBox.bottom = H;
		srcBox.front = 0;
		srcBox.back  = 1;

		ctx->CopySubresourceRegion(
		    staging.Get(),
		    0,
		    0, 0, 0,
		    frameTex.Get(),
		    0,
		    &srcBox);

		// Map only the small region
		D3D11_MAPPED_SUBRESOURCE map;
		hr = ctx->Map(staging.Get(), 0, D3D11_MAP_READ, 0, &map);

		if (SUCCEEDED(hr)) {
			uint8_t* row = (uint8_t*)map.pData;

			uint64_t sumR = 0, sumG = 0, sumB = 0;
			uint64_t count = uint64_t(W) * NUM_ROWS;

			for (UINT y = 0; y < NUM_ROWS; y++)
			{
				uint8_t* pix = row;
				for (UINT x = 0; x < W; x++)
				{
					uint8_t B = pix[0];
					uint8_t G = pix[1];
					uint8_t R = pix[2];

					sumR += R;
					sumG += G;
					sumB += B;

					pix += 4;
				}
				row += map.RowPitch;
			}

			uint8_t avgR = uint8_t(sumR / count);
			uint8_t avgG = uint8_t(sumG / count);
			uint8_t avgB = uint8_t(sumB / count);

			// Print or send to LED strip:
			std::cout << "AVG RGB = "
			          << (int)avgR << ", "
			          << (int)avgG << ", "
			          << (int)avgB << "\n";
			uint8_t pkt[] = { avgR, avgG, avgB };
			serialWrite(serial, pkt, sizeof(pkt));

			ctx->Unmap(staging.Get(), 0);
			Sleep(10);
		}

		dup->ReleaseFrame();
	}

	return EXIT_SUCCESS;
}
