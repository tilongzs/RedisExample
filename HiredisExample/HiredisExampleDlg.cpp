#include "pch.h"
#include "framework.h"
#include "HiredisExample.h"
#include "HiredisExampleDlg.h"
#include "afxdialogex.h"
#include <thread>

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

CHiredisExampleDlg::CHiredisExampleDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_REDISEXAMPLE_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CHiredisExampleDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_MSG, _editRecv);
	DDX_Control(pDX, IDC_IPADDRESS1, _redisIP);
	DDX_Control(pDX, IDC_EDIT_PORT, _editRedisPort);
	DDX_Control(pDX, IDC_BUTTON_CONN, _btnConn);
	DDX_Control(pDX, IDC_EDIT_Get, _editGet);
	DDX_Control(pDX, IDC_EDIT_SetKey, _editSetKey);
	DDX_Control(pDX, IDC_EDIT_SetValue, _editSetValue);
	DDX_Control(pDX, IDC_EDIT_SubChannel, _editChannel);
	DDX_Control(pDX, IDC_EDIT_SubMessage, _editPublishMessage);
	DDX_Control(pDX, IDC_EDIT_DB, _dbNum);
}

BEGIN_MESSAGE_MAP(CHiredisExampleDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_MESSAGE(WMSG_FUNCTION, &CHiredisExampleDlg::OnFunction)
	ON_BN_CLICKED(IDC_BUTTON_CONN, &CHiredisExampleDlg::OnBtnConn)
	ON_BN_CLICKED(IDC_BUTTON_Get, &CHiredisExampleDlg::OnBtnGet)
	ON_BN_CLICKED(IDC_BUTTON_Set, &CHiredisExampleDlg::OnBtnSet)
	ON_BN_CLICKED(IDC_BUTTON_Subscribe, &CHiredisExampleDlg::OnBtnSubscribe)
	ON_BN_CLICKED(IDC_BUTTON_Publish, &CHiredisExampleDlg::OnBtnPublish)
	ON_BN_CLICKED(IDC_BUTTON_PSubscribe, &CHiredisExampleDlg::OnBtnPSubscribe)
	ON_BN_CLICKED(IDC_BUTTON_DB_CHANGE, &CHiredisExampleDlg::OnBtnDbChange)
END_MESSAGE_MAP()


BOOL CHiredisExampleDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);

	_redisIP.SetAddress(127, 0, 0, 1);
	_editRedisPort.SetWindowText(L"6379");	
	_dbNum.SetWindowText(L"0");

	AppendMsg(L"启动");

	return TRUE; 
}

void CHiredisExampleDlg::OnPaint()
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

HCURSOR CHiredisExampleDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CHiredisExampleDlg::AppendMsg(const WCHAR* msg)
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

LRESULT CHiredisExampleDlg::OnFunction(WPARAM wParam, LPARAM lParam)
{
	TheadFunc* pFunc = (TheadFunc*)wParam;
	pFunc->Func();
	delete pFunc;

	return TRUE;
}

static void PushCallback(void* privdata, void* r)
{
	CHiredisExampleDlg* dlg = (CHiredisExampleDlg*)privdata;
	redisReply* reply = (redisReply*)r;
	
	if (reply->type != REDIS_REPLY_PUSH || reply->elements != 2 ||
		reply->element[1]->type != REDIS_REPLY_ARRAY ||
		reply->element[1]->element[0]->type != REDIS_REPLY_STRING)
	{
		dlg->AppendMsg(L"redisPushFn错误");
	}
	else
	{
		if (reply->type == REDIS_REPLY_ARRAY)
		{
			for (int i = 0; i < reply->elements; i++)
			{
				dlg->AppendMsg(CString(reply->element[i]->str));
			}
		}
		else
		{
			dlg->AppendMsg(CString(reply->str));
		}
	}

	freeReplyObject(reply);
}

static void enableClientTracking(redisContext* c) {
	redisReply* reply = (redisReply*)redisCommand(c, "HELLO 3");
	if (reply == NULL || c->err) {
		//panicAbort("NULL reply or server error (error: %s)", c->errstr);
	}

	if (reply->type != REDIS_REPLY_MAP) {
		fprintf(stderr, "Error: Can't send HELLO 3 command.  Are you sure you're ");
		fprintf(stderr, "connected to redis-server >= 6.0.0?\nRedis error: %s\n",
			reply->type == REDIS_REPLY_ERROR ? reply->str : "(unknown)");
		exit(-1);
	}

	freeReplyObject(reply);

	/* Enable client tracking */
	reply = (redisReply*)redisCommand(c, "CLIENT TRACKING ON");
	//assertReplyAndFree(c, reply, REDIS_REPLY_STATUS);
}

void CHiredisExampleDlg::OnBtnConn()
{
	BYTE ip1, ip2, ip3, ip4;
	_redisIP.GetAddress(ip1, ip2, ip3, ip4);
	string redisIP = str_format("%d.%d.%d.%d", ip1, ip2, ip3, ip4);

	CString strRedisPort;
	_editRedisPort.GetWindowText(strRedisPort);
	int redisPort = _wtoi(strRedisPort);

	redisOptions connectionOption = { 0 };
	struct timeval timeout = { 1, 500000 }; // 连接超时：1.5 seconds	
	REDIS_OPTIONS_SET_TCP(&connectionOption, redisIP.c_str(), redisPort);
	REDIS_OPTIONS_SET_PRIVDATA(&connectionOption, this, nullptr); // 设置回调函数参数
	connectionOption.connect_timeout = &timeout;
	connectionOption.push_cb = PushCallback;
	_redisContext = redisConnectWithOptions(&connectionOption);

//	_redisContext = redisConnect(redisIP.c_str(), redisPort);
	if (_redisContext == NULL || _redisContext->err)
	{
		AppendMsg(L"创建Redis连接失败");
		if (_redisContext)
		{
			AppendMsg(CString(_redisContext->errstr));
		}
	}
	else
	{
	//	enableClientTracking(_redisContext);

		_redisIP.EnableWindow(FALSE);
		_editRedisPort.EnableWindow(FALSE);
		_btnConn.EnableWindow(FALSE);

		AppendMsg(L"创建Redis连接完成");

		// 切换数据库
		OnBtnDbChange();
	}
}

void CHiredisExampleDlg::OnBtnSet()
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

	USES_CONVERSION;
	redisReply* reply = (redisReply*)redisCommand(_redisContext, "SET %s %s", W2A(strKey), W2A(strValue));
	if (reply)
	{
		AppendMsg(L"Set成功");
	}
	else
	{
		AppendMsg(L"Set失败");
		AppendMsg(A2W(_redisContext->errstr));
	}
	freeReplyObject(reply);
}

void CHiredisExampleDlg::OnBtnGet()
{
	CString strKey;
	_editGet.GetWindowText(strKey);
	if (strKey.IsEmpty())
	{
		MessageBox(L"strKey.IsEmpty()");
		return;
	}

	USES_CONVERSION;
	redisReply* reply = (redisReply*)redisCommand(_redisContext, "GET %s", W2A(strKey));
	if (reply)
	{
		if (reply->type != REDIS_REPLY_NIL)
		{
			AppendMsg(CString(reply->str));
		}
		else
		{
			AppendMsg(L"Get为空");
		}	
	}
	else
	{
		AppendMsg(L"Get失败");
		AppendMsg(A2W(_redisContext->errstr));
	}
	freeReplyObject(reply);
}

void CHiredisExampleDlg::OnBtnSubscribe()
{
	CString strTmp;
	_editChannel.GetWindowText(strTmp);
	if (strTmp.IsEmpty())
	{
		MessageBox(L"channel IsEmpty()");
		return;
	}

	thread([&, strTmp]
		{
			USES_CONVERSION;
			redisReply* reply = (redisReply*)redisCommand(_redisContext, "SUBSCRIBE %s", W2A(strTmp));
			if (reply)
			{
				freeReplyObject(reply);
				AppendMsg(L"SUBSCRIBE成功");
				while (redisGetReply(_redisContext, (void**)&reply) == REDIS_OK)
				{
					if (reply)
					{
						if (reply->type == REDIS_REPLY_ARRAY)
						{
							for (int i = 0; i < reply->elements; i++)
							{
								AppendMsg(CString(reply->element[i]->str));
							}
						}
						else
						{
							AppendMsg(CString(reply->str));
						}						
					}
					else
					{
						AppendMsg(L"SUBSCRIBE过程中遇到错误");
						AppendMsg(A2W(_redisContext->errstr));
					}

					freeReplyObject(reply);
				}
			}
			else
			{
				AppendMsg(L"SUBSCRIBE失败");
				AppendMsg(A2W(_redisContext->errstr));
			}
		}).detach();
}

void CHiredisExampleDlg::OnBtnPSubscribe()
{
	CString strTmp;
	_editChannel.GetWindowText(strTmp);
	if (strTmp.IsEmpty())
	{
		MessageBox(L"channel IsEmpty()");
		return;
	}

	thread([&, strTmp]
		{
			USES_CONVERSION;
			redisReply* reply = (redisReply*)redisCommand(_redisContext, "PSUBSCRIBE %s", W2A(strTmp));
			if (reply)
			{
				freeReplyObject(reply);
				AppendMsg(L"SUBSCRIBE成功");
				while (redisGetReply(_redisContext, (void**)&reply) == REDIS_OK)
				{
					if (reply)
					{
						if (reply->type == REDIS_REPLY_ARRAY)
						{
							for (int i = 0; i < reply->elements; i++)
							{
								AppendMsg(CString(reply->element[i]->str));
							}
						}
						else
						{
							AppendMsg(CString(reply->str));
						}
					}
					else
					{
						AppendMsg(L"PSUBSCRIBE过程中遇到错误");
						AppendMsg(A2W(_redisContext->errstr));
					}

					freeReplyObject(reply);
				}
			}
			else
			{
				AppendMsg(L"PSUBSCRIBE失败");
				AppendMsg(A2W(_redisContext->errstr));
			}
		}).detach();
}

void CHiredisExampleDlg::OnBtnPublish()
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

	redisReply* reply = (redisReply*)redisCommand(_redisContext, "PUBLISH %s %s", strChannel.c_str(), strMessage.c_str());
	if (reply)
	{
		if (reply->type != REDIS_REPLY_NIL)
		{
			AppendMsg(L"PUBLISH成功");
		}
		else
		{
			AppendMsg(L"PUBLISH为空");
		}
	}
	else
	{
		AppendMsg(L"PUBLISH失败");
		AppendMsg(A2W(_redisContext->errstr));
	}
	freeReplyObject(reply);
}

void CHiredisExampleDlg::OnBtnDbChange()
{
	CString dbNum;
	_dbNum.GetWindowText(dbNum);
	redisReply* reply = (redisReply*)redisCommand(_redisContext, "select %d", _wtoi(dbNum));
	if (reply)
	{
		AppendMsg(L"切换数据库成功");
	}
	else
	{
		USES_CONVERSION;
		AppendMsg(L"切换数据库失败");
		AppendMsg(A2W(_redisContext->errstr));
	}
	freeReplyObject(reply);
}
