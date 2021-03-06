
// MabinogiPackageToolDoc.h : CMabinogiPackageToolDoc 类的接口
//


#pragma once

#include <vector>
#include <memory>
#include "../MabinogiPackageResource/mabipackage.h"

using namespace std;
using namespace std::tr1;


class CPackEntry
{
public:
	shared_ptr< vector<byte> > GetData()
	{
		PPACKENTRY pEntry = pack_input_read_for_entry(m_pInput, m_index);
		shared_ptr< vector<byte> > buffer(new vector<byte>(pEntry->decompress_size));

		pack_input_read(m_pInput, &*buffer->begin(), buffer->size());

		return buffer;
	}

	PPACKENTRY GetEntry() 
	{
		return pack_input_get_entry(m_pInput, m_index);
	}
	
	/**
	 * 是否是文本内容，目前根据扩展名判断
	 */
	bool IsTextContent()
	{
		int index = m_strName.ReverseFind(TEXT('.'));
		if (index >= 0)
		{
			CString ext = m_strName.Mid( index + 1);
			ext.MakeLower();
			return ext == TEXT("xml") || ext == TEXT("txt");
		}
		return false;
	}

	/**
	 * 写入指定的文件
	 */
	void WriteToFile(LPCTSTR lpszFileName)
	{
		// 如果路径不存在则需要创建
		TCHAR szFullPath[MAX_PATH] = {0};
		lstrcpy(szFullPath, lpszFileName);
		for (int i = 0; szFullPath[i] != 0 && i < MAX_PATH; i++)
		{
			// 尝试创建所有文件夹
			if (szFullPath[i] == '\\' || szFullPath[i] == '/')
			{
				if (szFullPath[i - 1] != ':')
				{
					// 临时建立一个文件夹路径
					TCHAR old = szFullPath[i];
					szFullPath[i] = 0; // 直接截断字符串
					if (::GetFileAttributes(szFullPath) !=  FILE_ATTRIBUTE_DIRECTORY)
					{
						::CreateDirectory(szFullPath, NULL);
					}
					
					szFullPath[i] = old; // 恢复
				}
			}
		}

		shared_ptr<vector<byte> > spData = GetData();
		CFile file(lpszFileName, CFile::modeCreate | CFile::modeWrite);
		file.Write(&*spData->begin(), spData->size());
		file.Close();
	}

	CString m_strName;
	size_t m_index;
	PPACKINPUT m_pInput;

};

/**
 * Tree 节点
 */
class CPackFolder
{
public:
	void Clean()
	{
		m_entries.clear();
		m_children.clear();
	};

	shared_ptr<CPackFolder> FindOrCreateFolder( LPCTSTR name )
	{
		for (size_t i = 0;i < m_children.size();i++)
		{
			if (m_children.at(i)->m_strName == name)
			{
				return m_children.at(i);
			}
		}

		// 创建新的
		shared_ptr<CPackFolder> spFolder(new CPackFolder);
		spFolder->m_strName = name;
		spFolder->m_parent = this;
		m_children.push_back(spFolder);
		return spFolder;
	}

	CString m_strName;
	CPackFolder *m_parent;
	vector<shared_ptr<CPackFolder>> m_children;
	vector<shared_ptr<CPackEntry>> m_entries;
};

class CMabinogiPackageToolDoc : public CDocument
{
protected: // 仅从序列化创建
	CMabinogiPackageToolDoc();
	DECLARE_DYNCREATE(CMabinogiPackageToolDoc)

// 特性
public:

// 操作
public:

// 重写
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
#ifdef SHARED_HANDLERS
	virtual void InitializeSearchContent();
	virtual void OnDrawThumbnail(CDC& dc, LPRECT lprcBounds);
#endif // SHARED_HANDLERS

// 实现
public:
	virtual ~CMabinogiPackageToolDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	PPACKINPUT m_pPackInput;
	shared_ptr<CPackFolder> m_spRoot;

// 生成的消息映射函数
protected:
	DECLARE_MESSAGE_MAP()

#ifdef SHARED_HANDLERS
	// 用于为搜索处理程序设置搜索内容的 Helper 函数
	void SetSearchContent(const CString& value);
#endif // SHARED_HANDLERS
public:
	shared_ptr<CPackFolder> GetRoot();
	CPackFolder *m_pSelectedFolder;
	virtual void DeleteContents();
private:
	void Parse(void);
public:
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual void SetTitle(LPCTSTR lpszTitle);
};
