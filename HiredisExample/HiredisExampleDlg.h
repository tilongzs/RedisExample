#pragma once

#include <functional>
#include <memory>
#include "hiredis/hiredis.h"
using std::unique_ptr;
using std::function;

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

	HICON m_hIcon;
	CEdit _editRecv;
	CIPAddressCtrl _redisIP;
	CEdit _editRedisPort;
	CButton _btnConn;

	CEdit _editSetKey;
	CEdit _editSetValue;
	CEdit _editGet;
	CEdit _editChannel;
	CEdit _editPublishMessage;
	CEdit _dbNum;

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
public:
	void AppendMsg(const WCHAR* msg);
	afx_msg void OnBtnPSubscribe();
	afx_msg void OnBtnDbChange();
};
