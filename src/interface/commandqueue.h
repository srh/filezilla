#ifndef FILEZILLA_INTERFACE_COMMANDQUEUE_HEADER
#define FILEZILLA_INTERFACE_COMMANDQUEUE_HEADER

class CFileZillaEngine;
class CNotification;
class CState;
class CMainFrame;

#include <wx/event.h>

class CExclusiveHandler
{
public:
	virtual ~CExclusiveHandler() = default;
	virtual void ProcessNotification(CFileZillaEngine *pEngine, std::unique_ptr<CNotification>&& pNotification) = 0;
	virtual void OnExclusiveEngineRequestGranted(unsigned int requestId) = 0;
};

class CCommandQueue final
{
public:
	enum command_origin
	{
		any = -1,
		normal, // Most user actions
		recursiveOperation
	};

	CCommandQueue(CFileZillaEngine *pEngine, CMainFrame* pMainFrame, CState& state);

	void ProcessCommand(CCommand *pCommand, command_origin origin = normal);
	void ProcessNextCommand();
	bool Idle(command_origin origin = any) const;
	bool Cancel();
	bool Quit();
	void Finish(std::unique_ptr<COperationNotification> && pNotification);

	void RequestExclusiveEngine(CExclusiveHandler* exclusiveHandler, bool requestExclusive);

	CFileZillaEngine* GetEngineExclusive(unsigned int requestId);
	void ReleaseEngine(CExclusiveHandler *exclusiveHandler);
	bool EngineLocked() const { return m_exclusiveEngineLock; }

	void ProcessDirectoryListing(CDirectoryListingNotification const& listingNotification);

protected:
	void ProcessReply(int nReplyCode, Command commandId);

	void GrantExclusiveEngineRequest(CExclusiveHandler *exclusiveHandler);

	CFileZillaEngine* m_pEngine;
	CMainFrame* m_pMainFrame;
	CState& m_state;
	bool m_exclusiveEngineRequest{};
	bool m_exclusiveEngineLock{};
	unsigned int m_requestId{};
	static unsigned int m_requestIdCounter;
	CExclusiveHandler* m_exclusiveHandler{};

	// Used to make this class reentrance-safe
	int m_inside_commandqueue{};

	struct CommandInfo {
		CommandInfo() = default;
		CommandInfo(command_origin o, std::unique_ptr<CCommand> && c)
			: origin(o)
			, command(std::move(c))
		{}

		command_origin origin;
		std::unique_ptr<CCommand> command;
		bool didReconnect{};
	};
	std::deque<CommandInfo> m_CommandList;

	bool m_quit{};
};

#endif

