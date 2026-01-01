#pragma once
#define DIRECTINPUT_VERSION 0x0800
#include	<dinput.h>
#include	<Xinput.h>

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "xinput.lib")

class CDirectInput{
private:
	LPDIRECTINPUT8			m_dinput{};				// DirectInput8オブジェクト
	LPDIRECTINPUTDEVICE8	m_dikeyboard{};			// キーボードデバイス
	LPDIRECTINPUTDEVICE8	m_dimouse{};			// マウスデバイス
	char					m_keybuffer[256]{};		// キーボードバッファ
	char					m_oldkeybuffer[256]{};	// 前回の入力キーボードバッファ
	DIMOUSESTATE2			m_MouseState{};			// マウスの状態
	DIMOUSESTATE2			m_MouseStateTrigger{};	// マウスの状態
	POINT					m_MousePoint{};			// マウス座標
	int						m_width{};				// マウスのＸ座標最大
	int						m_height{};				// マウスのＹ座標最大
	HWND					m_hwnd{};
	XINPUT_STATE			m_pad{};				// コントローラの状態
	XINPUT_STATE			m_padOld{};				// 前回のコントローラの状態
	bool					m_padConnected = false;	// コントローラ接続状態
	CDirectInput() :m_dinput(nullptr), m_dikeyboard(nullptr), m_dimouse(nullptr) {
	}
public:

	CDirectInput(const CDirectInput&) = delete;
	CDirectInput& operator=(const CDirectInput&) = delete;
	CDirectInput(CDirectInput&&) = delete;
	CDirectInput& operator=(CDirectInput&&) = delete;

	static CDirectInput& GetInstance(){
		static CDirectInput Instance;
		return Instance;
	}

	~CDirectInput(){
		Dispose();
	}

	// すべての入力状態を更新
	void Update(void)
	{
		GetKeyBuffer();
		GetMouseState();
		GetPadState();
	}

	//----------------------------------
	// DirectInput 初期処理
	//
	//		P1 : インスタンス値
	//		P2 : ウインドウハンドル値
	//
	//	戻り値
	//		true : 初期化成功
	//		false : 初期化失敗
	//----------------------------------
	bool Init(HINSTANCE hInst,HWND hwnd,int width,int height){
		HRESULT	hr;
		hr = DirectInput8Create(hInst, DIRECTINPUT_VERSION, IID_IDirectInput8, (void **)&m_dinput, NULL);
		if(FAILED(hr)) {
			return false;
		}

		// キーボードデバイス生成
		m_dinput->CreateDevice(GUID_SysKeyboard, &m_dikeyboard, NULL);
		if(FAILED(hr)) {
			return false;
		}

		// データフォーマットの設定
		hr = m_dikeyboard->SetDataFormat(&c_dfDIKeyboard);
		if(FAILED(hr)) {
			return false;
		}
		
		// 協調レベルの設定
		hr = m_dikeyboard->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
		if(FAILED(hr)) {
			return false;
		}

		// マウスデバイス生成
		m_dinput->CreateDevice(GUID_SysMouse, &m_dimouse, NULL);
		if(FAILED(hr)) {
			return false;
		}

		// データフォーマットの設定
		hr = m_dimouse->SetDataFormat(&c_dfDIMouse2);
		if(FAILED(hr)) {
			return false;
		}
		
		// 協調レベルの設定
		hr = m_dimouse->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
		if(FAILED(hr)) {
			return false;
		}

		// デバイスの設定
		DIPROPDWORD diprop;
		diprop.diph.dwSize = sizeof(diprop);
		diprop.diph.dwHeaderSize = sizeof(diprop.diph);
		diprop.diph.dwObj = 0;
		diprop.diph.dwHow = DIPH_DEVICE;
		diprop.dwData = DIPROPAXISMODE_REL;							// 相対値モード
		m_dimouse->SetProperty(DIPROP_AXISMODE, &diprop.diph);		// 軸モードの設定


		DIPROPRANGE diprg;
		diprg.diph.dwSize = sizeof(diprg);
		diprg.diph.dwHeaderSize = sizeof(diprg.diph);
		diprg.diph.dwObj = DIJOFS_X;
		diprg.diph.dwHow = DIPH_BYOFFSET;
		diprg.lMin = 0;
		diprg.lMax = width - 1;

		m_dimouse->SetProperty(DIPROP_RANGE, &diprg.diph);		// Ｘ方向の範囲を指定
		diprg.diph.dwObj = DIJOFS_Y;
		diprg.diph.dwHow = DIPH_BYOFFSET;
		diprg.lMin = 0;
		diprg.lMax = height - 1;
		m_dimouse->SetProperty(DIPROP_RANGE, &diprg.diph);	// Ｙ方向の範囲を指定

		m_hwnd = hwnd;

		m_height = height;
		m_width  = width;

		return true;
	}

	//----------------------------------
	// マウス状態取得処理
	//----------------------------------
	void GetMouseState(){
		HRESULT	hr;

		DIMOUSESTATE2		mouseStateOld = m_MouseState;

		GetCursorPos(&m_MousePoint);
		ScreenToClient(m_hwnd, &m_MousePoint);

		// デバイスの認識
		hr = m_dimouse->Acquire();

		hr = m_dimouse->GetDeviceState(sizeof(m_MouseState),&m_MouseState);
		if (SUCCEEDED(hr)){
			for (int cnt = 0; cnt < 8; cnt++)
			{
				m_MouseStateTrigger.rgbButtons[cnt] = ((mouseStateOld.rgbButtons[cnt] ^ m_MouseState.rgbButtons[cnt]) & m_MouseState.rgbButtons[cnt]);
			}
		}
		else{
			if(hr == DIERR_INPUTLOST){
				// デバイスの認識
				hr = m_dimouse->Acquire();
			}
		}	
	}

	//----------------------------------
	// マウス状態取得処理
	//----------------------------------
	const DIMOUSESTATE2& GetMouseStateData() const {
		return m_MouseState;
	}

	//----------------------------------
	// マウスＸ座標取得処理
	//----------------------------------
	int GetMousePosX() const{
		return m_MousePoint.x;
	}

	//----------------------------------
	// マウスＹ座標取得処理
	//----------------------------------
	int GetMousePosY() const{
		return m_MousePoint.y;
	}

	//----------------------------------
	// マウス左ボタンチェック
	//----------------------------------
	bool GetMouseLButtonCheck() const{
		if(m_MouseState.rgbButtons[0] & 0x80){
			return true;
		}else{
			return false;	
		}
	}

	//----------------------------------
	// マウス右ボタンチェック
	//----------------------------------
	bool GetMouseRButtonCheck() const{
		if(m_MouseState.rgbButtons[1] & 0x80){
			return true;
		}else{
			return false;	
		}
	}

	//----------------------------------
	// マウス中央ボタンチェック
	//----------------------------------
	bool GetMouseCButtonCheck() const{
		if(m_MouseState.rgbButtons[2] & 0x80){
			return true;
		}else{
			return false;	
		}
	}

	//----------------------------------
	// マウス左ボタンチェック(トリガー)
	//----------------------------------
	bool GetMouseLButtonTrigger() const {
		if (m_MouseStateTrigger.rgbButtons[0] & 0x80) {
			return true;
		}
		else {
			return false;
		}
	}

	//----------------------------------
	// マウス右ボタンチェック(トリガー)
	//----------------------------------
	bool GetMouseRButtonTrigger() const {
		if (m_MouseStateTrigger.rgbButtons[1] & 0x80) {
			return true;
		}
		else {
			return false;
		}
	}

	//----------------------------------
	// マウス中央ボタンチェック(トリガー)
	//----------------------------------
	bool GetMouseCButtonTrigger() const {
		if (m_MouseStateTrigger.rgbButtons[2] & 0x80) {
			return true;
		}
		else {
			return false;
		}
	}

	//----------------------------------
	// キーボードバッファ取得処理
	//----------------------------------
	void GetKeyBuffer(){
		HRESULT	hr;
		// デバイスの認識
		hr = m_dikeyboard->Acquire();
		// 前回の状態を保存
		memcpy(&m_oldkeybuffer,m_keybuffer,sizeof(m_keybuffer));
		hr = m_dikeyboard->GetDeviceState(sizeof(m_keybuffer),(LPVOID)&m_keybuffer);
		if(hr == DIERR_INPUTLOST){
			// デバイスの認識
			hr = m_dikeyboard->Acquire();
		}
	}

	//----------------------------------
	// キーが押されているかどうかをチェックする
	//		p1 :　チェックしたいキー番号
	//	戻り値
	//		true : 指定されたキーが押されている
	//----------------------------------
	bool CheckKeyBuffer(int keyno){
		if(m_keybuffer[keyno] & 0x80){
			return true;
		}
		else{
			return false;
		}
	}

	//----------------------------------
	// キーが押されているかどうかをチェックする
	//		p1 :　チェックしたいキー番号(トリガー)
	//	戻り値
	//		true : 指定されたキーが押されている
	//----------------------------------
	bool CheckKeyBufferTrigger(int keyno){
		if(((m_keybuffer[keyno]^m_oldkeybuffer[keyno]) & m_keybuffer[keyno]) & 0x80){
			return true;
		}
		else{
			return false;
		}
	}

	//----------------------------------
	// DirectInput 終了処理
	//----------------------------------
	void Dispose(){
		if(m_dikeyboard!=nullptr){
			m_dikeyboard->Release();
		}
		if(m_dimouse!=nullptr){
			m_dimouse->Release();
		}
		if(m_dinput!=nullptr){
			m_dinput->Release();
		}
	}	

	//----------------------------------
	// カーソル位置を画面中央にする
	//----------------------------------
	void SetCursorPosition() {

		int x = GetSystemMetrics(SM_CXSCREEN);
		int y = GetSystemMetrics(SM_CYSCREEN);
		SetForegroundWindow(m_hwnd);
		SetActiveWindow(m_hwnd);
		SetFocus(m_hwnd);
		SetCursorPos(x/2, y/2);
	}

	//----------------------------------
	// コントローラ状態取得処理
	//----------------------------------
	void GetPadState(void)
	{
		m_padOld = m_pad;
		ZeroMemory(&m_pad, sizeof(m_pad));

		DWORD res = XInputGetState(0, &m_pad);
		m_padConnected = (res == ERROR_SUCCESS);
		if (!m_padConnected)
		{
			ZeroMemory(&m_pad, sizeof(m_pad));
			ZeroMemory(&m_padOld, sizeof(m_padOld));
		}
	}

	// コントローラ接続状態取得
	bool IsPadConnected(void) const { return m_padConnected; }

	// 左スティック（-1～1、デッドゾーン処理つき）
	DirectX::XMFLOAT2 GetLeftStick(void) const
	{
		if (!m_padConnected) { return DirectX::XMFLOAT2(0, 0); }

		float x = (float)m_pad.Gamepad.sThumbLX / 32767.0f;
		float y = (float)m_pad.Gamepad.sThumbLY / 32767.0f;

		// デッドゾーン
		constexpr float dz = 0.2f;
		float len = std::sqrt(x * x + y * y);
		if (len < dz) { return DirectX::XMFLOAT2(0, 0); }

		// 0～1に再正規化（倒し具合を滑らかに）
		float t = (len - dz) / (1.0f - dz);
		if (t > 1.0f) t = 1.0f;

		x /= len; y /= len;
		return DirectX::XMFLOAT2(x * t, y * t);
	}

	// 右スティック（-1～1、デッドゾーン処理つき）
	DirectX::XMFLOAT2 GetRightStick(void) const
	{
		if (!m_padConnected) { return DirectX::XMFLOAT2(0, 0); }

		float x = (float)m_pad.Gamepad.sThumbRX / 32767.0f;
		float y = (float)m_pad.Gamepad.sThumbRY / 32767.0f;

		// デッドゾーン
		constexpr float dz = 0.2f;
		float len = std::sqrt(x * x + y * y);
		if (len < dz) { return DirectX::XMFLOAT2(0, 0); }

		// 0～1に再正規化（倒し具合を滑らかに）
		float t = (len - dz) / (1.0f - dz);
		if (t > 1.0f) t = 1.0f;

		x /= len; y /= len;
		return DirectX::XMFLOAT2(x * t, y * t);
	}

	// ボタンの押下取得
	bool GetButtonPress(WORD btn) const
	{
		return m_padConnected && ((m_pad.Gamepad.wButtons & btn) != 0);
	}

	// ボタンのトリガー取得
	bool GetButtonTrigger(WORD btn) const
	{
		if (!m_padConnected) return false;
		bool now = (m_pad.Gamepad.wButtons & btn) != 0;
		bool old = (m_padOld.Gamepad.wButtons & btn) != 0;
		return now && !old;
	}

	// トリガーの倒し具合取得（0.0～1.0）
	float GetLeftTrigger(void) const
	{
		if (!m_padConnected) return 0.0f;
		return m_pad.Gamepad.bLeftTrigger / 255.0f;
	}
	float GetRightTrigger(void) const
	{
		if (!m_padConnected) return 0.0f;
		return m_pad.Gamepad.bRightTrigger / 255.0f;
	}

	// 何かしらの入力が今フレームトリガーされたか？
	bool AnyInputTriggered(void)
	{
		auto& in = CDirectInput::GetInstance();

		// キーボード：どれか1つでも「今フレーム押した」なら true
		for (int k = 0; k < 256; ++k)
		{
			if (in.CheckKeyBufferTrigger(k)) { return true; }
		}

		// マウス：主要3ボタン（必要なら増やす）
		if (in.GetMouseLButtonTrigger()) { return true; }
		if (in.GetMouseRButtonTrigger()) { return true; }
		if (in.GetMouseCButtonTrigger()) { return true; }

		// パッド：主要ボタン＋スティック＋トリガー
		if (in.IsPadConnected())
		{
			constexpr WORD buttons[] = {
				XINPUT_GAMEPAD_A, XINPUT_GAMEPAD_B, XINPUT_GAMEPAD_X, XINPUT_GAMEPAD_Y,
				XINPUT_GAMEPAD_START, XINPUT_GAMEPAD_BACK,
				XINPUT_GAMEPAD_LEFT_SHOULDER, XINPUT_GAMEPAD_RIGHT_SHOULDER,
				XINPUT_GAMEPAD_DPAD_UP, XINPUT_GAMEPAD_DPAD_DOWN,
				XINPUT_GAMEPAD_DPAD_LEFT, XINPUT_GAMEPAD_DPAD_RIGHT
			};

			// すべての主要ボタンをチェック
			for (WORD b : buttons)
			{
				if (in.GetButtonTrigger(b)) { return true; }
			}

			auto ls = in.GetLeftStick();
			// スティック倒し検出
			if (std::fabs(ls.x) > 0.2f || std::fabs(ls.y) > 0.2f) { return true; }

			if (in.GetLeftTrigger() > 0.2f) { return true; }
			if (in.GetRightTrigger() > 0.2f) { return true; }
		}

		return false;
	}

};
