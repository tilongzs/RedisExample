#include "pch.h"
#include "framework.h"
#include "RedisExample.h"
#include "RedisExampleDlg.h"
#include "afxdialogex.h"

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

template<typename ... Args>
std::string static str_format(const std::string& format, Args ... args)
{
	auto size_buf = std::snprintf(nullptr, 0, format.c_str(), args ...) + 1;
	std::unique_ptr<char[]> buf(new char[size_buf]);

	if (!buf)
		return std::string("");

	std::snprintf(buf.get(), size_buf, format.c_str(), args ...);
	return std::string(buf.get(), buf.get() + size_buf - 1);
}

#define WMSG_FUNCTION		WM_USER + 1

CRedisExampleDlg::CRedisExampleDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_REDISEXAMPLE_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CRedisExampleDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_MSG, _editRecv);
	DDX_Control(pDX, IDC_IPADDRESS1, _redisIP);
	DDX_Control(pDX, IDC_EDIT_PORT, _editRedisPort);
	DDX_Control(pDX, IDC_BUTTON_CONN, _btnConn);
	DDX_Control(pDX, IDC_EDIT_Get, _editGet);
	DDX_Control(pDX, IDC_EDIT_SetKey, _editSetKey);
	DDX_Control(pDX, IDC_EDIT_SetValue, _editSetValue);
	DDX_Control(pDX, IDC_EDIT_Sub, _editChannel);
	DDX_Control(pDX, IDC_EDIT_SubMessage, _editPublishMessage);
}

BEGIN_MESSAGE_MAP(CRedisExampleDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_MESSAGE(WMSG_FUNCTION, &CRedisExampleDlg::OnFunction)
	ON_BN_CLICKED(IDC_BUTTON_CONN, &CRedisExampleDlg::OnBtnConn)
	ON_BN_CLICKED(IDC_BUTTON_Get, &CRedisExampleDlg::OnBtnGet)
	ON_BN_CLICKED(IDC_BUTTON_Set, &CRedisExampleDlg::OnBtnSet)
	ON_BN_CLICKED(IDC_BUTTON_Subscribe, &CRedisExampleDlg::OnBtnSubscribe)
	ON_BN_CLICKED(IDC_BUTTON_Publish, &CRedisExampleDlg::OnBtnPublish)
	ON_BN_CLICKED(IDC_BUTTON_PSubscribe, &CRedisExampleDlg::OnBtnPSubscribe)
END_MESSAGE_MAP()


BOOL CRedisExampleDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);

	_redisIP.SetAddress(127, 0, 0, 1);
	_editRedisPort.SetWindowText(L"6379");	

	AppendMsg(L"启动");

	return TRUE; 
}

void CRedisExampleDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

HCURSOR CRedisExampleDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CRedisExampleDlg::AppendMsg(const WCHAR* msg)
{
	WCHAR* tmpMsg = new WCHAR[wcslen(msg) + 1];
	memset(tmpMsg, 0, sizeof(WCHAR) * (wcslen(msg) + 1));
	wsprintf(tmpMsg, msg);

	TheadFunc* pFunc = new TheadFunc;
	pFunc->Func = ([=]()
		{
			if (_editRecv.GetLineCount() > 100)
			{
				_editRecv.Clear();
			}

			CString curMsg;
			_editRecv.GetWindowTextW(curMsg);
			curMsg += "\r\n";

			CString strTime;
			SYSTEMTIME   tSysTime;
			GetLocalTime(&tSysTime);
			strTime.Format(L"%02ld:%02ld:%02ld.%03ld ",
				tSysTime.wHour, tSysTime.wMinute, tSysTime.wSecond, tSysTime.wMilliseconds);

			curMsg += strTime;
			curMsg += tmpMsg;
			_editRecv.SetWindowTextW(curMsg);
			_editRecv.LineScroll(_editRecv.GetLineCount());

			delete[] tmpMsg;
		});

	PostMessage(WMSG_FUNCTION, (WPARAM)pFunc);
}

LRESULT CRedisExampleDlg::OnFunction(WPARAM wParam, LPARAM lParam)
{
	TheadFunc* pFunc = (TheadFunc*)wParam;
	pFunc->Func();
	delete pFunc;

	return TRUE;
}

void CRedisExampleDlg::OnBtnConn()
{
	BYTE ip1, ip2, ip3, ip4;
	_redisIP.GetAddress(ip1, ip2, ip3, ip4);
	string redisIP = str_format("%d.%d.%d.%d", ip1, ip2, ip3, ip4);

	CString strRedisPort;
	_editRedisPort.GetWindowText(strRedisPort);
	int redisPort = _wtoi(strRedisPort);

	try
	{
		// 连接设置
		ConnectionOptions connection_options;
		connection_options.host = redisIP.c_str();
		connection_options.port = redisPort;
		connection_options.socket_timeout = std::chrono::milliseconds(1000);
		connection_options.connect_timeout = std::chrono::milliseconds(1000);
		//connection_options.password = "auth";

		// 连接池设置
		ConnectionPoolOptions pool_options;
		pool_options.size = 8;
		pool_options.wait_timeout = std::chrono::milliseconds(100);
		pool_options.connection_lifetime = std::chrono::minutes(10);

		// 创建连接（多线程安全）
		_redis = make_unique<Redis>(connection_options, pool_options);
// 		string strRedisConn = str_format("tcp://%s:%d", redisIP.c_str(), redisPort);
// 		_redis = make_unique<Redis>(strRedisConn.c_str());

		_redisIP.EnableWindow(FALSE);
		_editRedisPort.EnableWindow(FALSE);
		_btnConn.EnableWindow(FALSE);
		AppendMsg(L"创建Redis连接完成");
	}
	catch (const Error& e) 
	{
		AppendMsg(CString(e.what()));
	}
}

void CRedisExampleDlg::OnBtnSet()
{
	CString strKey;
	_editSetKey.GetWindowText(strKey);
	if (strKey.IsEmpty())
	{
		MessageBox(L"strKey.IsEmpty()");
		return;
	}

	CString strValue;
	_editSetValue.GetWindowText(strValue);
	if (strValue.IsEmpty())
	{
		MessageBox(L"strValue.IsEmpty()");
		return;
	}

	try
	{
		bool ret = _redis->set(CStringA(strKey).GetBuffer(), CStringA(strValue).GetBuffer());
		if (ret)
		{
			AppendMsg(L"Set成功");
		}
		else
		{
			AppendMsg(L"Set失败");
		}
	}
	catch (const Error& e) 
	{
		AppendMsg(CString(e.what()));
	}	
}

void CRedisExampleDlg::OnBtnGet()
{
	CString strKey;
	_editGet.GetWindowText(strKey);
	if (strKey.IsEmpty())
	{
		MessageBox(L"strKey.IsEmpty()");
		return;
	}

	try
	{
		auto val = _redis->get(CStringA(strKey).GetBuffer());
		if (val)
		{
			AppendMsg(CString(val->c_str()));
		}
		else
		{
			AppendMsg(L"Get为空");
		}
	}
	catch (const Error& e)
	{
		AppendMsg(CString(e.what()));
	}
}

void CRedisExampleDlg::OnBtnSubscribe()
{
	CString strTmp;
	_editChannel.GetWindowText(strTmp);
	if (strTmp.IsEmpty())
	{
		MessageBox(L"channel IsEmpty()");
		return;
	}
	AppendMsg(L"SUBSCRIBE成功");

	thread([&, strTmp]
		{
			auto sub = _redis->subscriber();
			sub.on_message([&](std::string channel, std::string msg)
				{
					string strLog = str_format("subscribe on_message channel:%s msg:%s", channel.c_str(), msg.c_str());

					USES_CONVERSION;
					AppendMsg(A2W(strLog.c_str()));
				});

			sub.subscribe(CStringA(strTmp).GetBuffer());

			while (true)
			{
				try
				{
					sub.consume();
				}
				catch (const TimeoutError& e) 
				{
					// 长时间没收到订阅的消息，会导致socket_timeout 
					continue;
				}
				catch (const Error& e)
				{
					AppendMsg(CString(e.what()));
				}
			}
		}).detach();	
}

void CRedisExampleDlg::OnBtnPSubscribe()
{
	CString strTmp;
	_editChannel.GetWindowText(strTmp);
	if (strTmp.IsEmpty())
	{
		MessageBox(L"channel IsEmpty()");
		return;
	}
	AppendMsg(L"PSUBSCRIBE成功");

	thread([&, strTmp]
		{
			auto sub = _redis->subscriber();
			sub.on_pmessage([&](std::string pattern, std::string channel, std::string msg)
				{
					string strLog = str_format("subscribe on_pmessage pattern:%s channel:%s msg:%s", pattern.c_str(), channel.c_str(), msg.c_str());

					USES_CONVERSION;
					AppendMsg(A2W(strLog.c_str()));
				});

			sub.psubscribe(CStringA(strTmp).GetBuffer());

			while (true)
			{
				try
				{
					sub.consume();
				}
				catch (const TimeoutError& e)
				{
					// 长时间没收到订阅的消息，会导致socket_timeout 
					continue;
				}
				catch (const Error& e)
				{
					AppendMsg(CString(e.what()));
				}
			}
		}).detach();
}

void CRedisExampleDlg::OnBtnPublish()
{
	USES_CONVERSION;
	CString strTmp;
	_editChannel.GetWindowText(strTmp);
	if (strTmp.IsEmpty())
	{
		MessageBox(L"channel IsEmpty()");
		return;
	}
	string strChannel = W2A(strTmp);

	_editPublishMessage.GetWindowText(strTmp);
	if (strTmp.IsEmpty())
	{
		MessageBox(L"message IsEmpty()");
		return;
	}
	string strMessage = W2A(strTmp);

	try
	{
		_redis->publish(strChannel.c_str(), strMessage.c_str());
	}
	catch (const Error& e)
	{
		AppendMsg(CString(e.what()));
	}
}
