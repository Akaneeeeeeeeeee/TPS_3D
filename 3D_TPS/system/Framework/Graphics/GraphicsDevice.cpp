#include "GraphicsDevice.h"
#include "../Window/Window.h"


//=======================================
//初期化処理
//=======================================
HRESULT GraphicsDevice::Init()
{
	HRESULT hr = S_OK;

	// デバイス、スワップチェーン作成
	DXGI_SWAP_CHAIN_DESC swapChainDesc{};
	swapChainDesc.BufferCount = 1; // バックバッファの数を1に設定（ダブルバッファリング）
	swapChainDesc.BufferDesc.Width = Window::GetInstance().GetWidth(); // バッファの幅をウィンドウサイズに合わせる
	swapChainDesc.BufferDesc.Height = Window::GetInstance().GetHeight(); // バッファの高さをウィンドウサイズに合わせる
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // バッファのピクセルフォーマットを設定
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60; // リフレッシュレートを設定（Hz）
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // バッファの使用用途を設定
	swapChainDesc.OutputWindow = Window::GetInstance().GetHandleWindow(); // スワップチェーンのターゲットウィンドウを設定
	swapChainDesc.SampleDesc.Count = 1; // マルチサンプリングの設定（アンチエイリアスのサンプル数とクオリティ）
	swapChainDesc.SampleDesc.Quality = 0; //同上
	swapChainDesc.Windowed = TRUE; // ウィンドウモード（フルスクリーンではなく、ウィンドウモードで実行）

	hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, NULL, 0,
		D3D11_SDK_VERSION, &swapChainDesc, m_pSwapChain.GetAddressOf(), m_pDevice.GetAddressOf(), &m_FeatureLevel, m_pDeviceContext.GetAddressOf());
	if (FAILED(hr)) return hr;

	// レンダーターゲットビュー作成
	ID3D11Texture2D* renderTarget{};
	hr = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&renderTarget);
	if (renderTarget != nullptr)m_pDevice->CreateRenderTargetView(renderTarget, NULL, m_pRenderTargetView.GetAddressOf());
	renderTarget->Release();

	// デプスステンシルバッファ作成
	ID3D11Texture2D* depthStencile{};
	D3D11_TEXTURE2D_DESC textureDesc{};
	textureDesc.Width = swapChainDesc.BufferDesc.Width;   // バッファの幅をスワップチェーンに合わせる
	textureDesc.Height = swapChainDesc.BufferDesc.Height; // バッファの高さをスワップチェーンに合わせる
	textureDesc.MipLevels = 1;                            // ミップレベルは1（ミップマップは使用しない）
	textureDesc.ArraySize = 1;                            // テクスチャの配列サイズ（通常1）
	textureDesc.Format = DXGI_FORMAT_D16_UNORM;           // フォーマットは16ビットの深度バッファを使用
	textureDesc.SampleDesc = swapChainDesc.SampleDesc;    // スワップチェーンと同じサンプル設定
	textureDesc.Usage = D3D11_USAGE_DEFAULT;              // 使用方法はデフォルト（GPUで使用）
	textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;     // 深度ステンシルバッファとして使用
	textureDesc.CPUAccessFlags = 0;                       // CPUからのアクセスは不要
	textureDesc.MiscFlags = 0;                            // その他のフラグは設定なし

	hr = m_pDevice->CreateTexture2D(&textureDesc, NULL, &depthStencile);
	if (FAILED(hr)) return hr;

	// デプスステンシルビュー作成
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
	depthStencilViewDesc.Format = textureDesc.Format; // デプスステンシルバッファのフォーマットを設定
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D; // ビューの次元を2Dテクスチャとして設定（2Dテクスチャ用のデプスステンシルビュー）
	depthStencilViewDesc.Flags = 0; // 特別なフラグは設定しない（デフォルトの動作）
	if (depthStencile != nullptr)m_pDevice->CreateDepthStencilView(depthStencile, &depthStencilViewDesc, m_pDepthStencilView.GetAddressOf());
	depthStencile->Release();

	m_pDeviceContext->OMSetRenderTargets(1, m_pRenderTargetView.GetAddressOf(), m_pDepthStencilView.Get()); // レンダーターゲットとデプスステンシルビューを設定

	// ビューポート設定
	D3D11_VIEWPORT viewport;
	viewport.Width = (FLOAT)Window::GetInstance().GetWidth();   // ビューポートの幅
	viewport.Height = (FLOAT)Window::GetInstance().GetHeight(); // ビューポートの高さ
	viewport.MinDepth = 0.0f;                          // 深度範囲の最小値
	viewport.MaxDepth = 1.0f;                          // 深度範囲の最大値
	viewport.TopLeftX = 0;                             // ビューポートの左上隅のX座標
	viewport.TopLeftY = 0;                             // ビューポートの左上隅のY座標
	m_pDeviceContext->RSSetViewports(1, &viewport);


	// ラスタライザステート設定
	D3D11_RASTERIZER_DESC rasterizerDesc{};
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	//rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
	// -----------カリング設定（ポリゴンのどこ（表裏など）を表示しないか）--------------------
	//rasterizerDesc.CullMode = D3D11_CULL_BACK;
	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	//rasterizerDesc.CullMode = D3D11_CULL_FRONT;
	rasterizerDesc.DepthClipEnable = TRUE;
	rasterizerDesc.MultisampleEnable = FALSE;

	ID3D11RasterizerState* rs;
	hr = m_pDevice->CreateRasterizerState(&rasterizerDesc, &rs);
	if (FAILED(hr)) return hr;

	m_pDeviceContext->RSSetState(rs);


	// ここから下は別クラスに分割

	// ブレンド ステート生成
	//D3D11_BLEND_DESC BlendDesc;
	//ZeroMemory(&BlendDesc, sizeof(BlendDesc));							// BlendDesc構造体をゼロで初期化し、メモリをクリア
	//BlendDesc.AlphaToCoverageEnable = FALSE;							// アルファ・トゥ・カバレッジを無効化（透明度をカバレッジとして利用しない）
	//BlendDesc.IndependentBlendEnable = TRUE;							// 各レンダーターゲットに対して個別のブレンド設定を有効化
	//BlendDesc.RenderTarget[0].BlendEnable = FALSE;						// ブレンドを無効に設定（不透明な描画）
	//BlendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;			// ソース（描画するピクセル）のアルファ値を使用
	//BlendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;	// デスティネーション（既存のピクセル）の逆アルファ値を使用
	//BlendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;				// ソースとデスティネーションを加算する操作
	//BlendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;			// ソースのアルファ値をそのまま使用
	//BlendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;		// デスティネーションのアルファ値を無視
	//BlendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;		// アルファ値に対して加算操作を行う
	//BlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL; // レンダーターゲットのカラーチャンネル書き込みマスク

	//hr = m_Device->CreateBlendState(&BlendDesc, m_pBlendState[0].GetAddressOf());
	//if (FAILED(hr)) return;

	//// ブレンド ステート生成 (アルファ ブレンド用)
	////BlendDesc.AlphaToCoverageEnable = TRUE;
	//BlendDesc.RenderTarget[0].BlendEnable = TRUE;
	//hr = m_Device->CreateBlendState(&BlendDesc, m_BlendState[1].GetAddressOf());
	//if (FAILED(hr)) return;

	//// ブレンド ステート生成 (加算合成用)
	//BlendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	//hr = m_Device->CreateBlendState(&BlendDesc, m_BlendState[2].GetAddressOf());
	//if (FAILED(hr)) return;

	//// ブレンド ステート生成 (減算合成用)
	//BlendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_REV_SUBTRACT;
	//hr = m_Device->CreateBlendState(&BlendDesc, m_BlendState[3].GetAddressOf());
	//if (FAILED(hr)) return;

	//SetBlendState(BS_ALPHABLEND);


	// デプスステンシルステート設定
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc{};
	depthStencilDesc.DepthEnable = TRUE;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	// ------------デバッグ用(深度バッファに関わらず全部映すモード) ----------------
	//depthStencilDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
	// --------------------------------------------------------------------
	depthStencilDesc.StencilEnable = FALSE;

	hr = m_pDevice->CreateDepthStencilState(&depthStencilDesc, m_pDepthStateEnable.GetAddressOf()); //深度有効ステート
	if (FAILED(hr)) return hr;

	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	hr = m_pDevice->CreateDepthStencilState(&depthStencilDesc, m_pDepthStateDisable.GetAddressOf()); //深度無効ステート
	if (FAILED(hr)) return hr;

	m_pDeviceContext->OMSetDepthStencilState(m_pDepthStateEnable.Get(), NULL);

	// サンプラーステート設定
	D3D11_SAMPLER_DESC samplerDesc{};
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MaxAnisotropy = 4;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	ID3D11SamplerState* samplerState{};
	hr = m_pDevice->CreateSamplerState(&samplerDesc, &samplerState);
	if (FAILED(hr)) 	return hr;;

	m_pDeviceContext->PSSetSamplers(0, 1, &samplerState);
	return hr;

	//// 定数バッファ生成
	//D3D11_BUFFER_DESC bufferDesc{};
	//bufferDesc.ByteWidth = sizeof(Matrix);
	//bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	//bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	//bufferDesc.CPUAccessFlags = 0;
	//bufferDesc.MiscFlags = 0;
	//bufferDesc.StructureByteStride = sizeof(float);
	//// 0番目の定数バッファにworld行列
	//hr = m_Device->CreateBuffer(&bufferDesc, NULL, m_WorldBuffer.GetAddressOf());
	//m_DeviceContext->VSSetConstantBuffers(0, 1, m_WorldBuffer.GetAddressOf());
	//if (FAILED(hr)) return;
	//// 1番目の定数バッファにview行列
	//hr = m_Device->CreateBuffer(&bufferDesc, NULL, m_ViewBuffer.GetAddressOf());
	//m_DeviceContext->VSSetConstantBuffers(1, 1, m_ViewBuffer.GetAddressOf());
	//if (FAILED(hr)) return;
	//// 2番目の定数バッファにProjection行列
	//hr = m_Device->CreateBuffer(&bufferDesc, NULL, m_ProjectionBuffer.GetAddressOf());
	//m_DeviceContext->VSSetConstantBuffers(2, 1, m_ProjectionBuffer.GetAddressOf());
	//if (FAILED(hr)) return;
	//// 3番目の定数バッファに光源情報
	//bufferDesc.ByteWidth = sizeof(LIGHT);
	//hr = m_Device->CreateBuffer(&bufferDesc, NULL, m_LightBuffer.GetAddressOf());
	//m_DeviceContext->VSSetConstantBuffers(3, 1, m_LightBuffer.GetAddressOf());
	//if (FAILED(hr)) return;

	//// ライト初期化
	//LIGHT light{};
	//light.Enable = true;
	//light.Direction = Vector4(0.5f, -1.0f, 0.8f, 0.8f);		// 方向
	//light.Direction.Normalize();
	//light.Diffuse = Color(1.0f, 1.0f, 1.0f, 1.0f);			// 平行光源の強さと色
	//light.Ambient = Color(0.75f, 0.75f, 0.75f, 1.0f);		// 環境光の強さと色
	//SetLight(light);

	//// 4番目の定数バッファにマテリアル情報
	//bufferDesc.ByteWidth = sizeof(MATERIAL);
	//hr = m_Device->CreateBuffer(&bufferDesc, NULL, m_MaterialBuffer.GetAddressOf());
	//m_DeviceContext->VSSetConstantBuffers(4, 1, m_MaterialBuffer.GetAddressOf());
	//m_DeviceContext->PSSetConstantBuffers(4, 1, m_MaterialBuffer.GetAddressOf());
	//if (FAILED(hr)) return;

	//// マテリアル初期化
	//MATERIAL material;
	//material.Diffuse = Color(1.0f, 1.0f, 1.0f, 1.0f);
	//material.Ambient = Color(1.0f, 1.0f, 1.0f, 1.0f);
	//SetMaterial(material);

	//bufferDesc.ByteWidth = sizeof(Matrix);
	//hr = m_Device->CreateBuffer(&bufferDesc, NULL, m_TextureBuffer.GetAddressOf());
	//m_DeviceContext->VSSetConstantBuffers(5, 1, m_TextureBuffer.GetAddressOf());
	//if (FAILED(hr))  return;

	//// UV初期化
	//SetUV(0, 0, 1, 1);

}

//=======================================
//終了処理
//=======================================
void GraphicsDevice::Uninit()
{
	// ステートをクリア
	m_pDeviceContext->ClearState();
}

//=======================================
//描画開始
//=======================================
void GraphicsDevice::StartRender()
{
	// 全消し
	float clearColor[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
	m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView.Get(), clearColor);
	m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
}

//=======================================
//描画終了
//=======================================
void GraphicsDevice::FinishRender()
{
	// 入れ替え
	m_pSwapChain->Present(1, 0);
}

////=======================================
//// 深度ステンシルの有効・無効を設定
////=======================================
//void GraphicsDevice::SetDepthEnable(bool Enable)
//{
//	if (Enable)
//	{
//		// 深度テストを有効にするステンシルステートをセット
//		m_DeviceContext->OMSetDepthStencilState(m_DepthStateEnable.Get(), NULL);
//	}
//	else
//	{
//		// 深度テストを無効にするステンシルステートをセット
//		m_DeviceContext->OMSetDepthStencilState(m_DepthStateDisable.Get(), NULL);
//	}
//}
//
////=======================================
//// アルファテストとカバレッジ（ATC）の有効・無効を設定
////=======================================
//void GraphicsDevice::SetATCEnable(bool Enable)
//{
//	// ブレンドファクター（透明度などの調整に使用）
//	float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
//
//	if (Enable)
//	{
//		// アルファテストとカバレッジ (ATC) を有効にするブレンドステートをセット
//		m_DeviceContext->OMSetBlendState(m_BlendStateATC.Get(), blendFactor, 0xffffffff);
//	}
//	else
//	{
//		// 通常のブレンドステートをセット
//		m_DeviceContext->OMSetBlendState(m_BlendState[0].Get(), blendFactor, 0xffffffff);
//	}
//}
//
////=======================================
////
////=======================================
//void GraphicsDevice::SetWorldViewProjection2D()
//{
//	Matrix world = Matrix::Identity;			// 単位行列にする
//	world = world.Transpose();			// 転置
//	m_DeviceContext->UpdateSubresource(m_WorldBuffer.Get(), 0, NULL, &world, 0, 0);
//
//	Matrix view = Matrix::Identity;			// 単位行列にする
//	view = view.Transpose();			// 転置
//	m_DeviceContext->UpdateSubresource(m_ViewBuffer.Get(), 0, NULL, &view, 0, 0);
//
//	// 2D描画を左上原点にする
//	Matrix projection = DirectX::XMMatrixOrthographicOffCenterLH(
//		0.0f,
//		static_cast<float>(Application::GetWidth()),	// ビューボリュームの最小Ｘ
//		static_cast<float>(Application::GetHeight()),	// ビューボリュームの最小Ｙ
//		0.0f,											// ビューボリュームの最大Ｙ
//		0.0f,
//		1.0f);
//
//	projection = projection.Transpose();
//
//	m_DeviceContext->UpdateSubresource(m_ProjectionBuffer.Get(), 0, NULL, &projection, 0, 0);
//}
//
////=======================================
//// ワールド行列を設定
////=======================================
//void GraphicsDevice::SetWorldMatrix(Matrix* WorldMatrix)
//{
//	Matrix world;
//	world = WorldMatrix->Transpose(); // 転置
//
//	// ワールド行列をGPU側へ送る
//	m_DeviceContext->UpdateSubresource(m_WorldBuffer.Get(), 0, NULL, &world, 0, 0);
//}
//
////=======================================
//// ビュー行列を設定
////=======================================
//void GraphicsDevice::SetViewMatrix(Matrix* ViewMatrix)
//{
//	Matrix view;
//	view = ViewMatrix->Transpose(); // 転置
//
//	// ビュー行列をGPU側へ送る
//	m_DeviceContext->UpdateSubresource(m_ViewBuffer.Get(), 0, NULL, &view, 0, 0);
//}
//
////=======================================
//// プロジェクション行列を設定
////=======================================
//void GraphicsDevice::SetProjectionMatrix(Matrix* ProjectionMatrix)
//{
//	Matrix projection;
//	projection = ProjectionMatrix->Transpose(); // 転置
//
//	// プロジェクション行列をGPU側へ送る
//	m_DeviceContext->UpdateSubresource(m_ProjectionBuffer.Get(), 0, NULL, &projection, 0, 0);
//}
//
////=======================================
//// ライトを設定
////=======================================
//void GraphicsDevice::SetLight(LIGHT Light) {
//	// ライトの設定をGPU側に送る
//	m_DeviceContext->UpdateSubresource(m_LightBuffer.Get(), 0, NULL, &Light, 0, 0);
//}
//
////=======================================
//// マテリアルを設定
////=======================================
//void GraphicsDevice::SetMaterial(MATERIAL Material) {
//	// マテリアルの設定をGPU側に送る
//	m_DeviceContext->UpdateSubresource(m_MaterialBuffer.Get(), 0, NULL, &Material, 0, 0);
//}
//
////=======================================
//// UV情報を設定
////=======================================
//void GraphicsDevice::SetUV(float u, float v, float uw, float vh) {
//	// UV行列作成
//	Matrix mat = Matrix::CreateScale(uw, vh, 1.0f);
//	mat *= Matrix::CreateTranslation(u, v, 0.0f).Transpose();
//
//	m_DeviceContext->UpdateSubresource(m_TextureBuffer.Get(), 0, NULL, &mat, 0, 0);
//}
//
////=======================================
////頂点シェーダー作成
////=======================================
//void GraphicsDevice::CreateVertexShader(ID3D11VertexShader** VertexShader, ID3D11InputLayout** VertexLayout, const char* FileName)
//{
//	FILE* file; // ファイルを開くためのポインタ
//	long int fsize; // ファイルサイズを格納する変数
//	fopen_s(&file, FileName, "rb"); // シェーダーファイルをReadBinaryモードで開く
//	assert(file); // ファイルが正常に開けたかどうかを確認
//
//	fsize = _filelength(_fileno(file)); // ファイルのサイズを取得
//	unsigned char* buffer = new unsigned char[fsize]; // ファイルサイズに基づいてバッファを確保
//	fread(buffer, fsize, 1, file); // ファイルからバッファにデータを読み込む
//	fclose(file); // 読み込み完了後、ファイルを閉じる
//
//	// デバイスを使って頂点シェーダーを作成
//	m_Device->CreateVertexShader(buffer, fsize, NULL, VertexShader);
//
//	// 頂点レイアウト（入力レイアウト）の定義
//	D3D11_INPUT_ELEMENT_DESC layout[] =
//	{
//		// 頂点の位置情報（3つのfloat値）
//		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,		0,	0,		D3D11_INPUT_PER_VERTEX_DATA, 0 },
//		// 頂点の法線ベクトル情報（3つのfloat値）
//		{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,		0,	4 * 3,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
//		// 頂点の色情報（4つのfloat値：RGBA）
//		{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0,	4 * 6,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
//		// 頂点のテクスチャ座標情報（2つのfloat値）
//		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,			0,	4 * 10, D3D11_INPUT_PER_VERTEX_DATA, 0 }
//	};
//	UINT numElements = ARRAYSIZE(layout); // レイアウトの要素数を取得
//
//	// デバイスを使って頂点レイアウトを作成
//	m_Device->CreateInputLayout(layout, numElements, buffer, fsize, VertexLayout);
//
//	delete[] buffer; // バッファのメモリを解放
//}
//
////=======================================
////ピクセルシェーダー作成
////=======================================
//void GraphicsDevice::CreatePixelShader(ID3D11PixelShader** PixelShader, const char* FileName)
//{
//	FILE* file; // ファイルを開くためのポインタ
//	long int fsize; // ファイルサイズを格納する変数
//	fopen_s(&file, FileName, "rb"); // シェーダーファイルをReadBinaryモードで開く
//	assert(file); // ファイルが正常に開けたかどうかを確認
//
//	fsize = _filelength(_fileno(file)); // ファイルのサイズを取得
//	unsigned char* buffer = new unsigned char[fsize]; // ファイルサイズに基づいてバッファを確保
//	fread(buffer, fsize, 1, file); // ファイルからバッファにデータを読み込む
//	fclose(file); // 読み込み完了後、ファイルを閉じる
//
//	// デバイスを使ってピクセルシェーダーを作成
//	m_Device->CreatePixelShader(buffer, fsize, NULL, PixelShader);
//
//	delete[] buffer; // バッファのメモリを解放
//}
