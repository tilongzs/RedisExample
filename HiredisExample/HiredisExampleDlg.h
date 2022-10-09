#pragma once

#include <functional>
#include <memory>
#include "hiredis/hiredis.h"
#include <atomic>
#include <mutex>
using std::unique_ptr;
using std::function;
using std::atomic;
using std::mutex;
using std::shared_ptr;
using std::list;
using std::string;

class CHiredisExampleDlg : public CDialogEx
{
public:
	CHiredisExampleDlg(CWnd* pParent = nullptr);

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_REDISEXAMPLE_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

	struct TheadFunc
	{
		function<void()> Func;
	};

	class RedisController
	{
	public:
		string connect(const char* ip, int port);
		bool reconnect();
		mutex					mtxBusy;
		redisContext*			redisContext;
		atomic<bool>			isConnect;
	};

	HICON m_hIcon;
	CEdit _editRecv;
	CIPAddressCtrl _redisIP;
	CEdit _editRedisPort;
	CButton _btnConn;
	CButton _btnReconn;

	string	_ip;
	int		_port;

	CEdit _editSetKey;
	CEdit _editSetValue;
	CEdit _editGet;
	CEdit _editChannel;
	CEdit _editPublishMessage;

	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBtnConn();
	afx_msg void OnBtnSet();
	afx_msg void OnBtnGet();
	afx_msg void OnBtnSubscribe();
	afx_msg void OnBtnPublish();
	DECLARE_MESSAGE_MAP()
		
	LRESULT OnFunction(WPARAM wParam, LPARAM lParam);

private:	
	redisContext*  _redisContext = nullptr; // 非线程安全
	mutex	_mtxRedisControllerList;
	list<shared_ptr<RedisController>> _redisControllerList;
public:
	void AppendMsg(const WCHAR* msg);
	afx_msg void OnBtnPSubscribe();
	shared_ptr<RedisController> GetConnectedRedisController();
	afx_msg void OnBtnReconn();
};
