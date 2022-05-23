#pragma once
#include "redis++/redis++.h"
#include "redis++/redis.h"
using namespace sw::redis;

#include <functional>
#include <memory>
using std::unique_ptr;
using std::function;

class CRedisExampleDlg : public CDialogEx
{
public:
	CRedisExampleDlg(CWnd* pParent = nullptr);

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_REDISEXAMPLE_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);


	struct TheadFunc
	{
		function<void()> Func;
	};
protected:
	HICON m_hIcon;
	CEdit _editRecv;
	CIPAddressCtrl _redisIP;
	CEdit _editRedisPort;
	CButton _btnConn;

	CEdit _editSetKey;
	CEdit _editSetValue;
	CEdit _editGet;
	
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

	void AppendMsg(const WCHAR* msg);
	LRESULT OnFunction(WPARAM wParam, LPARAM lParam);

private:	
	unique_ptr<Redis> _redis;
public:
	afx_msg void OnBtnConn();	
	afx_msg void OnBtnSet();
	afx_msg void OnBtnGet();	
};
