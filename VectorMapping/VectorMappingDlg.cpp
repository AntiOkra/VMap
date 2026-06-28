
// VectorMappingDlg.cpp : 実装ファイル
//

#include "stdafx.h"
#include "VectorMapping.h"
#include "VectorMappingDlg.h"
#include "afxdialogex.h"
#include <math.h>
#include <locale.h>
#include <algorithm>

#include "AdxModel.h"
#include "AdxElementSet.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// アプリケーションのバージョン情報に使われる CAboutDlg ダイアログ

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// ダイアログ データ
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

// 実装
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CVectorMappingDlg ダイアログ



CVectorMappingDlg::CVectorMappingDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_VECTORMAPPING_DIALOG, pParent)
	, adx_path_(_T(""))
	, nastran_path_(_T(""))
	, normal_pressure_path_(_T(""))
	, tangent_pressure_path_(_T(""))
	, search_distance_(0)
	, force_output_path_(_T(""))
	, m_str_selcount(_T(""))
	, m_StrForceX(_T("-"))
	, m_StrForceY(_T("-"))
	, m_StrForceZ(_T("-"))
	, mapping_ratio_(1)
	, mapping_ratio_x_(1)
	, mapping_ratio_y_(1)
	, mapping_ratio_z_(1)
	, use_xyz_ratio_(FALSE)
	, m_StrForceNorm(_T("-"))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CVectorMappingDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_ADX, adx_path_);
	DDX_Text(pDX, IDC_EDIT_NASTRAN, nastran_path_);
	DDX_Text(pDX, IDC_EDIT_NORMAL, normal_pressure_path_);
	DDX_Text(pDX, IDC_EDIT_TANGENT, tangent_pressure_path_);
	DDX_Text(pDX, IDC_EDIT_UPR_LIMIT, search_distance_);
	DDV_MinMaxDouble(pDX, search_distance_, 1.0, 30.0);
	DDX_Text(pDX, IDC_EDIT_OUT_FORCE, force_output_path_);
	DDX_Control(pDX, IDC_LIST_ADX, m_ListElementSet);
	DDX_Control(pDX, IDC_LIST_PARTS, m_ListParts);
	DDX_Text(pDX, IDC_STATIC_SEL_COUNT, m_str_selcount);
	DDV_MaxChars(pDX, m_str_selcount, 20);
	DDX_Text(pDX, IDC_EDIT_FORCE_X, m_StrForceX);
	DDX_Text(pDX, IDC_EDIT_FORCE_Y, m_StrForceY);
	DDX_Text(pDX, IDC_EDIT_FORCE_Z, m_StrForceZ);
	DDX_Text(pDX, IDC_EDIT_FORCE_X2, mapping_ratio_);
	DDX_Text(pDX, IDC_EDIT_MAPPING_RATIO, mapping_ratio_);
	DDX_Text(pDX, IDC_EDIT_MAPPING_RATIO_X, mapping_ratio_x_);
	DDX_Text(pDX, IDC_EDIT_MAPPING_RATIO_Y, mapping_ratio_y_);
	DDX_Text(pDX, IDC_EDIT_MAPPING_RATIO_Z, mapping_ratio_z_);
	DDX_Check(pDX, IDC_CHECK_XYZ, use_xyz_ratio_);
	DDX_Text(pDX, IDC_EDIT_FORCE_NORM, m_StrForceNorm);
}

BEGIN_MESSAGE_MAP(CVectorMappingDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_ADX, &CVectorMappingDlg::OnBnClickedButtonAdx)
	ON_BN_CLICKED(IDC_BUTTON_NASTRAN, &CVectorMappingDlg::OnBnClickedButtonNastran)
	ON_BN_CLICKED(IDC_BUTTON_NORMAL, &CVectorMappingDlg::OnBnClickedButtonNormal)
	ON_BN_CLICKED(IDC_BUTTON_TANGENT, &CVectorMappingDlg::OnBnClickedButtonTangent)
	ON_BN_CLICKED(IDC_BUTTON_OUT, &CVectorMappingDlg::OnBnClickedButtonOut)
	ON_BN_CLICKED(IDOK, &CVectorMappingDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON_ADXREAD, &CVectorMappingDlg::OnBnClickedButtonAdxread)
	ON_BN_CLICKED(IDC_BUTTON_EXEC, &CVectorMappingDlg::OnBnClickedButtonExec)
	ON_BN_CLICKED(IDCANCEL, &CVectorMappingDlg::OnBnClickedCancel)
	ON_LBN_SELCHANGE(IDC_LIST_ADX, &CVectorMappingDlg::OnLbnSelchangeListAdx)
	ON_BN_CLICKED(IDC_BUTTON_CLEAR_SELECT, &CVectorMappingDlg::OnBnClickedButtonClearSelect)
	ON_BN_CLICKED(IDC_BUTTON_FORCE_CALC, &CVectorMappingDlg::OnBnClickedButtonForceCalc)
	ON_EN_KILLFOCUS(IDC_EDIT_MAPPING_RATIO, &CVectorMappingDlg::OnEnKillfocusEditMappingRatio)
	ON_EN_KILLFOCUS(IDC_EDIT_UPR_LIMIT, &CVectorMappingDlg::OnEnKillfocusEditUprLimit)
	ON_EN_KILLFOCUS(IDC_EDIT_MAPPING_RATIO_X, &CVectorMappingDlg::OnEnKillfocusEditMappingRatioX)
	ON_EN_KILLFOCUS(IDC_EDIT_MAPPING_RATIO_Y, &CVectorMappingDlg::OnEnKillfocusEditMappingRatioY)
	ON_EN_KILLFOCUS(IDC_EDIT_MAPPING_RATIO_Z, &CVectorMappingDlg::OnEnKillfocusEditMappingRatioZ)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_LIST_PARTS, &CVectorMappingDlg::OnLvnColumnclickListParts)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_PARTS, &CVectorMappingDlg::OnLvnItemchangedListParts)
	ON_BN_CLICKED(IDC_CHECK_XYZ, &CVectorMappingDlg::OnBnClickedCheckXyz)
	ON_EN_KILLFOCUS(IDC_EDIT_NASTRAN, &CVectorMappingDlg::OnEnKillfocusEditNastran)
	ON_EN_KILLFOCUS(IDC_EDIT_NORMAL, &CVectorMappingDlg::OnEnKillfocusEditNormal)
	ON_EN_KILLFOCUS(IDC_EDIT_TANGENT, &CVectorMappingDlg::OnEnKillfocusEditTangent)
	ON_EN_KILLFOCUS(IDC_EDIT_OUT_FORCE, &CVectorMappingDlg::OnEnKillfocusEditOutForce)
END_MESSAGE_MAP()


// CVectorMappingDlg メッセージ ハンドラー

BOOL CVectorMappingDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// "バージョン情報..." メニューをシステム メニューに追加します。

	// IDM_ABOUTBOX は、システム コマンドの範囲内になければなりません。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// このダイアログのアイコンを設定します。アプリケーションのメイン ウィンドウがダイアログでない場合、
	//  Framework は、この設定を自動的に行います。
	SetIcon(m_hIcon, TRUE);			// 大きいアイコンの設定
	SetIcon(m_hIcon, FALSE);		// 小さいアイコンの設定

	// TODO: 初期化をここに追加します。
	search_distance_ = 10.0;
	mapping_ratio_ = 1.0;
	mapping_ratio_x_ = 1.0;
	mapping_ratio_y_ = 1.0;
	mapping_ratio_z_ = 1.0;
	m_str_selcount = "0";

	m_StrForceX = "-";
	m_StrForceY = "-";
	m_StrForceZ = "-";
	m_StrForceNorm = "-";

	use_xyz_ratio_ = FALSE;

	DWORD dwStyle = m_ListParts.GetExtendedStyle();
	//dwStyle |= LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES;
	dwStyle |= LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT| LVS_EX_HEADERINALLVIEWS;
	m_ListParts.SetExtendedStyle(dwStyle);

	m_ListParts.InsertColumn(0, _T("NAME"), LVCFMT_LEFT, 200);
	m_ListParts.InsertColumn(1, _T("ID"), LVCFMT_RIGHT, 100);
	m_ListParts.InsertColumn(2, _T("COMMENT"), LVCFMT_LEFT, 200);

	log_is_open_ = false;

	UpdateData(FALSE);

	ControlEnable(false);

	return TRUE;  // フォーカスをコントロールに設定した場合を除き、TRUE を返します。
}

void CVectorMappingDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// ダイアログに最小化ボタンを追加する場合、アイコンを描画するための
//  下のコードが必要です。ドキュメント/ビュー モデルを使う MFC アプリケーションの場合、
//  これは、Framework によって自動的に設定されます。

void CVectorMappingDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 描画のデバイス コンテキスト

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// クライアントの四角形領域内の中央
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// アイコンの描画
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// ユーザーが最小化したウィンドウをドラッグしているときに表示するカーソルを取得するために、
//  システムがこの関数を呼び出します。
HCURSOR CVectorMappingDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CVectorMappingDlg::OnBnClickedButtonAdx()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	CString         filter("ADX Files (*.adx)|*.adx||");
	CFileDialog     selDlg(TRUE, NULL, NULL, OFN_HIDEREADONLY, filter);

	if (selDlg.DoModal() == IDOK)
	{
		adx_path_ = selDlg.GetPathName();

		m_ListElementSet.ResetContent();
		m_ListParts.DeleteAllItems();

		CString sel_string;
		sel_string.Format(_T("%d"), 0);
		m_str_selcount = sel_string;

		UpdateData(FALSE);
	}

	return;
}


void CVectorMappingDlg::OnBnClickedButtonNastran()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	CString         filter("NASTRAN Files (*.nas)|*.nas||");
	CFileDialog     selDlg(TRUE, NULL, NULL, OFN_HIDEREADONLY, filter);

	if (selDlg.DoModal() == IDOK)
	{
		nastran_path_ = selDlg.GetPathName();

		m_StrForceX = "-";
		m_StrForceY = "-";
		m_StrForceZ = "-";
		m_StrForceNorm = "-";

		UpdateData(FALSE);
	}

	return;
}


void CVectorMappingDlg::OnBnClickedButtonNormal()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	CString         filter("NormalContactPressureVector Files (*.asc)|*.asc||");
	CFileDialog     selDlg(TRUE, NULL, NULL, OFN_HIDEREADONLY, filter);

	if (selDlg.DoModal() == IDOK)
	{
		normal_pressure_path_ = selDlg.GetPathName();

		m_StrForceX = "-";
		m_StrForceY = "-";
		m_StrForceZ = "-";
		m_StrForceNorm = "-";

		UpdateData(FALSE);
	}

	return;
}


void CVectorMappingDlg::OnBnClickedButtonTangent()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	CString         filter("TangentContactPressureVector Files (*.asc)|*.asc||");
	CFileDialog     selDlg(TRUE, NULL, NULL, OFN_HIDEREADONLY, filter);

	if (selDlg.DoModal() == IDOK)
	{
		tangent_pressure_path_ = selDlg.GetPathName();

		m_StrForceX = "-";
		m_StrForceY = "-";
		m_StrForceZ = "-";
		m_StrForceNorm = "-";

		UpdateData(FALSE);
	}

	return;
}


void CVectorMappingDlg::OnBnClickedButtonOut()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	// [名前を付けて保存]ダイアログ
	CString         filter("出力ファイル Files (*.adx)|*.adx||");
	CFileDialog selDlg(FALSE, _T("adx"), _T("force"), OFN_OVERWRITEPROMPT, filter);

	if (selDlg.DoModal() == IDOK) {
		force_output_path_ = selDlg.GetPathName();
		UpdateData(FALSE);
	}

	return;
}


void LogFileName(CString& adx_path, CString& log_path);

void CVectorMappingDlg::OnBnClickedOk()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。

	CString log_path;
	LogFileName(adx_path_, log_path);
	CStdioFile LogFile;

	// 出力ファイル
	BOOL ret;
	ret = LogFile.Open(log_path, CFile::modeCreate | CFile::modeWrite | CFile::shareDenyNone |CFile::typeText);
	if (ret == FALSE) {
		return;
	}

	nastran_model_.log_file_ = &LogFile;
	surface_node_map_.log_file_ = &LogFile;

	if (nastran_model_.ReadNastranFile(nastran_path_) != 0) {
		AfxMessageBox(_T("Nastranファイル読み込めませんでした"));
		return;
	}

	if (nastran_model_.ReadNormalVectorFile(normal_pressure_path_) != 0) {
		AfxMessageBox(_T("NormalPressureVectorファイル読み込めませんでした"));
		return;
	}

	if (nastran_model_.ReadTangentVectorFile(tangent_pressure_path_) != 0) {
		AfxMessageBox(_T("TangentPressureVectorファイル読み込めませんでした"));
		return;
	}

	if (nastran_model_.ForceCalc() != 0) {
		AfxMessageBox(_T("Nastranファイルの荷重を計算できませんでした"));
		return;
	}

	// マッピングするパーツを取得
	std::vector<int> v_sel;
	CStringArray v_name;

	for (int i = 0; i<m_ListElementSet.GetCount(); i++) {
		if (m_ListElementSet.GetSel(i)) {
			CString& name = (adx_model_.element_sets_[i])->adx_name_;
			v_name.Add(name);
		}
	}

	// Adxファイルの表面ノードを取得
	if (adx_model_.ExtractSurfaceNode(v_name, surface_node_map_) != 0) {
		AfxMessageBox(_T("表面ノードを取得できませんでした"));
		return;
	}
	else {
		// AfxMessageBox(_T("表面ノードを取得できました"));
	}

	// エリアマップ
	if (surface_node_map_.BuildSpatialGrid() != 0) {
		AfxMessageBox(_T("ノードマップ作成エラーです"));
		return;
	}

	CString dump_path;

	/* Dump ( debug用 )
	dump_path = _T("nastran_debug.txt");
	nastran_model_.Dump(dump_path);

	dump_path = _T("dump_map_debug.txt");
	surface_node_map_.Dump(dump_path);
	*/

	CMzPoint ratio;

	if (use_xyz_ratio_) {
		ratio.x = mapping_ratio_x_;
		ratio.y = mapping_ratio_y_;
		ratio.z = mapping_ratio_z_;
	}
	else {
		ratio.x = mapping_ratio_;
		ratio.y = mapping_ratio_;
		ratio.z = mapping_ratio_;
	}

	// マッピング
	if (surface_node_map_.MapForces(nastran_model_, search_distance_, ratio) != 0) {
		AfxMessageBox(_T("マッピング実行時エラーです"));
		return;
	}

	// Forceデータ出力
	CString process = _T("TEST");
	if (surface_node_map_.ExportAdxForce(force_output_path_,process) != 0) {
		AfxMessageBox(_T("Forceファイル出力ができませんでした"));
	}

	CString buf;

	CMzPoint nastran_force;

	nastran_model_.GetTotalForce(nastran_force);
	buf.Format(_T("NASTRAN_FORCE:%10.3f,%10.3f,%10.3f\n"), nastran_force.x,nastran_force.y,nastran_force.z);
	LogFile.WriteString(buf);

	buf.Format(_T("MAPPED_FORCE:%10.3f,%10.3f,%10.3f\n"), surface_node_map_.mapped_force_.x, surface_node_map_.mapped_force_.y, surface_node_map_.mapped_force_.z);
	LogFile.WriteString(buf);

	buf.Format(_T("LOSSED_FORCE:%10.3f,%10.3f,%10.3f\n"), surface_node_map_.loss_force_.x, surface_node_map_.loss_force_.y, surface_node_map_.loss_force_.z);
	LogFile.WriteString(buf);

	LogFile.Close();

	CDialogEx::OnOK();
}

void DumpFileName(CString& nas_path, CString& e_path, CString& n_path)
{
	int ipos = nas_path.ReverseFind('.');
	CString folder = nas_path.Left(ipos);

	e_path.Format(_T("%s_element.csv"), folder);
	n_path.Format(_T("%s\\_node.csv"), folder);
}

void LogFileName(CString& adx_path, CString& log_path)
{
	int ipos = adx_path.ReverseFind('\\');
	CString folder = adx_path.Left(ipos);

	log_path.Format(_T("%s\\mapping_log.txt"), folder);
}

void ModFileName(CString& org_path, char period_char, CString& replace_str, CString& mod_path)
{
	int ipos = org_path.ReverseFind(period_char);
	CString org_left = org_path.Left(ipos);

	mod_path.Format(_T("%s%s"), org_left, replace_str);
}

bool IsBlankPath(const CString& path)
{
	CString trimmed(path);
	trimmed.Trim();
	return trimmed.IsEmpty();
}

bool FileExists(const CString& path)
{
	if (IsBlankPath(path)) {
		return false;
	}

	CFileStatus status;
	return CFile::GetStatus(path, status) == TRUE;
}

void CVectorMappingDlg::OnBnClickedButtonAdxread()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	UpdateData(TRUE);

	if (!FileExists(adx_path_)) {
		AfxMessageBox(_T("ADXファイルを指定してください"));
		return;
	}

	m_ListElementSet.ResetContent();
	m_ListParts.DeleteAllItems();

	// ログファイルオープン
	CString log_path;
	LogFileName(adx_path_, log_path);

	CFileStatus fstatus;
	BOOL ret;

	if ( ! log_is_open_ ) {
		ret = log_file_.Open(log_path, CFile::modeCreate | CFile::modeWrite | CFile::shareDenyNone | CFile::typeText);
		if (ret == FALSE) {
			return;
		}
		log_is_open_ = true;
	}

	// LOCALE設定
	setlocale(LC_CTYPE, "jpn");

	adx_model_.RemoveAll();

	if (adx_model_.Read(adx_path_) == 0) {

		if (adx_model_.Activate() != 0) {
			ControlEnable(false);
			AfxMessageBox(_T("ADXファイル構成できませんでした"));
			return;
		}

		// IDでソート
		//sort(adx_model_.element_sets_.begin(), adx_model_.element_sets_.end(), AdxElementSet::Compare);

		CString s_item;
		for (int i = 0; i < adx_model_.element_sets_.size(); i++) {
			AdxElementSet& es = *(adx_model_.element_sets_[i]);
			s_item.Format(_T("%-30s %-20s"), es.adx_name_.Trim(), es.user_name_.Trim());
			//m_ListElementSet.InsertString(i, es.adx_name_ + " " + es.user_name_);
			m_ListElementSet.InsertString(i, s_item);
		}

		for (int i = 0; i < adx_model_.element_sets_.size(); i++) {
			ListInsertElementSet(i,*(adx_model_.element_sets_[i]));
		}

		ControlEnable(true);

		AfxMessageBox(_T("ADXファイル読み込み完了"));
	}
	else {

		ControlEnable(false);

		AfxMessageBox(_T("ADXファイル読み込めませんでした"));
	}


	CString buf;

	buf.Format(_T("-------------------------------------------------------\n"));
	log_file_.WriteString(buf);

	buf.Format(_T("ADX_FILE読み込みました:%s\n"), adx_path_);
	log_file_.WriteString(buf);

	buf.Format(_T("-------------------------------------------------------\n"));
	log_file_.WriteString(buf);

}

void CVectorMappingDlg::ControlEnable(bool to_enable)
{
	CWnd* myWnd;

	myWnd = GetDlgItem(IDC_EDIT_OUT_FORCE);
	myWnd->EnableWindow(to_enable);
	myWnd = GetDlgItem(IDC_BUTTON_FORCE);
	myWnd->EnableWindow(to_enable);

	myWnd = GetDlgItem(IDC_BUTTON_CLEAR_SELECT);
	myWnd->EnableWindow(to_enable);
	myWnd = GetDlgItem(IDC_LIST_ADX);
	myWnd->EnableWindow(to_enable);
	myWnd = GetDlgItem(IDC_BUTTON_EXEC);
	myWnd->EnableWindow(to_enable);
}
bool CVectorMappingDlg::EnsureLogFileOpen()
{
	if (log_is_open_) {
		return true;
	}

	CString log_path;
	LogFileName(adx_path_, log_path);
	if (log_file_.Open(log_path, CFile::modeCreate | CFile::modeWrite | CFile::shareDenyNone | CFile::typeText) == FALSE) {
		AfxMessageBox(_T("ログファイルを作成できませんでした"));
		return false;
	}

	log_is_open_ = true;
	return true;
}

bool CVectorMappingDlg::ValidatePressureInputFiles(bool require_output_path)
{
	if (!FileExists(nastran_path_)) {
		AfxMessageBox(_T("Nastranファイルを指定してください"));
		return false;
	}
	if (!FileExists(normal_pressure_path_)) {
		AfxMessageBox(_T("NormalPressureVectorファイルを指定してください"));
		return false;
	}
	if (!FileExists(tangent_pressure_path_)) {
		AfxMessageBox(_T("TangentPressureVectorファイルを指定してください"));
		return false;
	}
	if (require_output_path && IsBlankPath(force_output_path_)) {
		AfxMessageBox(_T("出力Forceファイルを指定してください"));
		return false;
	}

	return true;
}

bool CVectorMappingDlg::ValidateMappingInputs()
{
	if (adx_model_.IsEmpty()) {
		AfxMessageBox(_T("先にADXファイルを読み込んでください"));
		return false;
	}

	return ValidatePressureInputFiles(true);
}

int CVectorMappingDlg::LoadNastranInputs(NastranModel& nastran)
{
	if (nastran.ReadNastranFile(nastran_path_) != 0) {
		AfxMessageBox(_T("Nastranファイル読み込めませんでした"));
		return 1;
	}

	if (nastran.ReadNormalVectorFile(normal_pressure_path_) != 0) {
		AfxMessageBox(_T("NormalPressureVectorファイル読み込めませんでした"));
		return 1;
	}

	if (nastran.ReadTangentVectorFile(tangent_pressure_path_) != 0) {
		AfxMessageBox(_T("TangentPressureVectorファイル読み込めませんでした"));
		return 1;
	}

	if (nastran.ForceCalc() != 0) {
		AfxMessageBox(_T("Nastranファイルの荷重を計算できませんでした"));
		return 1;
	}

	return 0;
}

int CVectorMappingDlg::CollectCheckedElementSetNames(CStringArray& names)
{
	names.RemoveAll();

	HWND hwndList = GetDlgItem(IDC_LIST_PARTS)->m_hWnd;
	int nCount = m_ListParts.GetItemCount();

	for (int nItem = 0; nItem < nCount; nItem++) {
		if (ListView_GetCheckState(hwndList, nItem) == TRUE) {
			AdxElementSet* tmp_es = (AdxElementSet*)(m_ListParts.GetItemData(nItem));
			names.Add(tmp_es->adx_name_);
		}
	}

	return static_cast<int>(names.GetCount());
}

CMzPoint CVectorMappingDlg::CurrentMappingRatio() const
{
	if (use_xyz_ratio_) {
		return CMzPoint(mapping_ratio_x_, mapping_ratio_y_, mapping_ratio_z_);
	}

	return CMzPoint(mapping_ratio_, mapping_ratio_, mapping_ratio_);
}

void CVectorMappingDlg::UpdateCheckedPartCount()
{
	CString sel_string;
	CStringArray selected_names;
	sel_string.Format(_T("%d"), CollectCheckedElementSetNames(selected_names));
	m_str_selcount = sel_string;
}

void CVectorMappingDlg::OnBnClickedButtonExec()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	if (!UpdateData(TRUE)) {
		return;
	}
	if (!ValidateMappingInputs()) {
		return;
	}
	if (!EnsureLogFileOpen()) {
		return;
	}

	//　初期化
	nastran_model_.RemoveAll();
	surface_node_map_.Clear();

	nastran_model_.log_file_ = &log_file_;
	surface_node_map_.log_file_ = &log_file_;

	if (LoadNastranInputs(nastran_model_) != 0) {
		return;
	}

	CStringArray v_name;
	if (CollectCheckedElementSetNames(v_name) == 0) {
		AfxMessageBox(_T("マッピングするパーツを選択してください"));
		return;
	}

	// Adxファイルの表面ノードを取得
	if (adx_model_.ExtractSurfaceNode(v_name, surface_node_map_) != 0) {
		AfxMessageBox(_T("表面ノードを取得できませんでした"));
		return;
	}

	// エリアマップ
	if (surface_node_map_.BuildSpatialGrid() != 0) {
		AfxMessageBox(_T("ノードマップ作成エラーです"));
		return;
	}

	CMzPoint ratio = CurrentMappingRatio();

	if (surface_node_map_.MapForces(nastran_model_, search_distance_, ratio) != 0) {
		AfxMessageBox(_T("マッピング実行時エラーです"));
		return;
	}

	// Forceデータ出力
	CString process = _T("AdvcStep-99999");
	if (surface_node_map_.ExportAdxForce(force_output_path_, process) != 0) {
		AfxMessageBox(_T("Forceファイル出力ができませんでした"));
		return;
	}

	// Dump(Debug用)
	CString n_path;
	CString mod_str = _T("_N.csv");
	ModFileName(nastran_path_, '.', mod_str, n_path);
	nastran_model_.Dump_N(n_path);

	CString e_path;
	mod_str = _T("_E.csv");
	ModFileName(nastran_path_, '.', mod_str, e_path);
	nastran_model_.Dump_E(e_path);

	CString m_path;
	mod_str = _T("_map.csv");
	ModFileName(force_output_path_, '.', mod_str, m_path);
	surface_node_map_.DumpNode(m_path);

	CString buf;

	buf.Format(_T("-------------------------------------------------------\n"));
	log_file_.WriteString(buf);

	buf.Format(_T("NASTRAN FILE:%s\n"), nastran_path_);
	log_file_.WriteString(buf);

	buf.Format(_T("PAM_STAMP NormalVectorFile :%s\n"), normal_pressure_path_);
	log_file_.WriteString(buf);

	buf.Format(_T("PAM_STAMP TangentVectorFile:%s\n"), tangent_pressure_path_);
	log_file_.WriteString(buf);

	buf.Format(_T("Adx Force Mapping File     :%s\n"), force_output_path_);
	log_file_.WriteString(buf);

	buf.Format(_T("マッピング時 係数          : X(%8.3f) Y(%8.3f) Z(%8.3f)\n"), ratio.x, ratio.y, ratio.z);
	log_file_.WriteString(buf);

	CMzPoint nastran_force;

	nastran_model_.GetTotalForce(nastran_force);
	buf.Format(_T("NASTRAN_荷重(kN) (元)           :%10.3f,%10.3f,%10.3f\n"), nastran_force.x / 1000.0, nastran_force.y / 1000.0, nastran_force.z / 1000.0);
	log_file_.WriteString(buf);

	buf.Format(_T("NASTRAN_荷重(kN) (係数処理後)   :%10.3f,%10.3f,%10.3f\n"), nastran_force.x / 1000.0 * ratio.x, nastran_force.y / 1000.0 * ratio.y, nastran_force.z / 1000.0 * ratio.z);
	log_file_.WriteString(buf);

	buf.Format(_T("マッピングされた荷重(kN)        :%10.3f,%10.3f,%10.3f\n"), surface_node_map_.mapped_force_.x / 1000.0, surface_node_map_.mapped_force_.y / 1000.0, surface_node_map_.mapped_force_.z / 1000.0);
	log_file_.WriteString(buf);

	buf.Format(_T("マッピングされなかった荷重(kN)  :%10.3f,%10.3f,%10.3f\n"), surface_node_map_.loss_force_.x / 1000.0, surface_node_map_.loss_force_.y / 1000.0, surface_node_map_.loss_force_.z / 1000.0);
	log_file_.WriteString(buf);

	buf.Format(_T("-------------------------------------------------------\n"));
	log_file_.WriteString(buf);

	log_file_.Flush();

	nastran_model_.RemoveAll();
	surface_node_map_.Clear();

	AfxMessageBox(_T("マッピング完了しました"));
}

void CVectorMappingDlg::OnBnClickedCancel()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	if ( log_is_open_ ) {
		log_file_.Close();
	}

	CDialogEx::OnCancel();
}


void CVectorMappingDlg::OnLbnSelchangeListAdx()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	int sel_count = 0;
	for (int i = 0; i<m_ListElementSet.GetCount(); i++) {
		if (m_ListElementSet.GetSel(i)) {
			sel_count++;
		}
	}

	//
	CString sel_string;
	sel_string.Format(_T("%d"), sel_count);
	m_str_selcount = sel_string;

	UpdateData(FALSE);
}


void CVectorMappingDlg::OnBnClickedButtonClearSelect()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	for (int i = 0; i < m_ListElementSet.GetCount(); i++) {
		m_ListElementSet.SetSel(i, FALSE);
	}

	HWND hwndList = GetDlgItem(IDC_LIST_PARTS)->m_hWnd;

	for (int i = 0; i < m_ListParts.GetItemCount(); i++) {
		ListView_SetCheckState(hwndList, i,0);
	}

	m_str_selcount = "0";
	UpdateData(FALSE);

}


void CVectorMappingDlg::OnBnClickedButtonForceCalc()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	if (!UpdateData(TRUE)) {
		return;
	}
	if (!ValidatePressureInputFiles(false)) {
		return;
	}

	NastranModel nas_test;
	if (LoadNastranInputs(nas_test) != 0) {
		return;
	}

	CMzPoint pam_force;

	nas_test.GetTotalForce(pam_force);

	m_StrForceX.Format(_T("%11.3f"), pam_force.x / 1000.0);
	m_StrForceY.Format(_T("%11.3f"), pam_force.y / 1000.0);
	m_StrForceZ.Format(_T("%11.3f"), pam_force.z / 1000.0);
	m_StrForceNorm.Format(_T("%11.3f"), sqrt(pam_force.x * pam_force.x + pam_force.y * pam_force.y + pam_force.z * pam_force.z) / 1000.0);

	UpdateData(FALSE);
}

void CVectorMappingDlg::ListInsertElementSet(int no_row, AdxElementSet &es)
{
	/*
	m_ListParts.InsertItem(0, _T("A"));
	m_ListParts.SetItemText(0, 1, L"1");
	m_ListParts.SetItemText(0, 2, L"3");
	*/

	CString sid;
	sid.Format(_T("%d"), es.id_);

	//m_ListParts.InsertItem(no_row, es.adx_name_);

	int insert_row = m_ListParts.InsertItem(LVIF_TEXT | LVIF_PARAM, no_row, es.adx_name_,0, 0, 0, (LPARAM)&(es));

	m_ListParts.SetItemText(insert_row, 1, sid);
	m_ListParts.SetItemText(insert_row, 2, es.user_name_);

	//UpdateData(TRUE);
}




void CVectorMappingDlg::OnLvnColumnclickListParts(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: ここにコントロール通知ハンドラー コードを追加します。

	// 何列目がクリックされたか取得
	m_sortParam.m_nIndex = pNMLV->iSubItem;

	// ソートを行う
	m_ListParts.SortItems(CompareData, (LPARAM)&m_sortParam);

	// 次回は昇順・降順を逆にする
	m_sortParam.m_bSort = !m_sortParam.m_bSort;


	*pResult = 0;
}

int CVectorMappingDlg::CompareData(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	int result = 0;

	AdxElementSet* d1 = (AdxElementSet*)lParam1;
	AdxElementSet* d2 = (AdxElementSet*)lParam2;

	CSortParameter* sortParam = (CSortParameter*)lParamSort;
	//CSortParameter* sortParam = &m_sortParam;

	if (sortParam->m_nIndex == 0) {
		result = d1->adx_name_.Compare(d2->adx_name_);
	}
	else if (sortParam->m_nIndex == 1) {
		if (d1->id_ > d2->id_) {
			result = 1;
		}
		else if (d1->id_ < d2->id_) {
			result = -1;
		}
		else {
			result = 0;
		}
	}
	else if (sortParam->m_nIndex == 2) {
		result = d1->user_name_.Compare(d2->user_name_);
	}

	if (sortParam->m_bSort) {
		result *= -1;
	}

	return result;
}

void CVectorMappingDlg::OnLvnItemchangedListParts(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: ここにコントロール通知ハンドラー コードを追加します。

	UpdateCheckedPartCount();
	UpdateData(FALSE);

	*pResult = 0;
}

void CVectorMappingDlg::OnBnClickedCheckXyz()
{

	CEdit *e;

	UpdateData(TRUE);

	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	if (use_xyz_ratio_) {
		e = (CEdit*)GetDlgItem(IDC_EDIT_MAPPING_RATIO); e->EnableWindow(FALSE);
		e = (CEdit*)GetDlgItem(IDC_EDIT_MAPPING_RATIO_X); e->EnableWindow(TRUE);
		e = (CEdit*)GetDlgItem(IDC_EDIT_MAPPING_RATIO_Y); e->EnableWindow(TRUE);
		e = (CEdit*)GetDlgItem(IDC_EDIT_MAPPING_RATIO_Z); e->EnableWindow(TRUE);
	}
	else {
		e = (CEdit*)GetDlgItem(IDC_EDIT_MAPPING_RATIO); e->EnableWindow(TRUE);
		e = (CEdit*)GetDlgItem(IDC_EDIT_MAPPING_RATIO_X); e->EnableWindow(FALSE);
		e = (CEdit*)GetDlgItem(IDC_EDIT_MAPPING_RATIO_Y); e->EnableWindow(FALSE);
		e = (CEdit*)GetDlgItem(IDC_EDIT_MAPPING_RATIO_Z); e->EnableWindow(FALSE);
	}
}


void CVectorMappingDlg::OnEnKillfocusEditNastran()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	UpdateData(TRUE);
}


void CVectorMappingDlg::OnEnKillfocusEditNormal()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	UpdateData(TRUE);
}


void CVectorMappingDlg::OnEnKillfocusEditTangent()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	UpdateData(TRUE);
}


void CVectorMappingDlg::OnEnKillfocusEditOutForce()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	UpdateData(TRUE);
}


void CVectorMappingDlg::OnEnKillfocusEditMappingRatio()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	UpdateData(TRUE);
}


void CVectorMappingDlg::OnEnKillfocusEditUprLimit()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	UpdateData(TRUE);
}


void CVectorMappingDlg::OnEnKillfocusEditMappingRatioX()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	UpdateData(TRUE);
}


void CVectorMappingDlg::OnEnKillfocusEditMappingRatioY()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	UpdateData(TRUE);
}


void CVectorMappingDlg::OnEnKillfocusEditMappingRatioZ()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	UpdateData(TRUE);
}