#ifndef D3DTIMER_H
#define D3DTIMER_H

class Timer
{
	ID3D11Query*	mDisjoint;
	ID3D11Query*	mStart;
	ID3D11Query*	mStop;

	ID3D11Device*			mDevice;
	ID3D11DeviceContext*	mDeviceContext;

public:
	explicit Timer(ID3D11Device* d3dDevice, ID3D11DeviceContext* d3dDeviceContext);
	~Timer();

	void Start();
	void Stop();

	double GetTime();
};
#endif 