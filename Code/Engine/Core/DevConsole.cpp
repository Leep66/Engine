#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/Timer.hpp"
#include "Engine/Renderer/Renderer.hpp"



extern DevConsole* g_theDevConsole;
extern EventSystem* g_theEventSystem;
extern InputSystem* g_theInput;
extern Renderer* g_theRenderer;

const Rgba8 DevConsole::ERROR_COLOR = Rgba8(255, 0, 0, 255);
const Rgba8 DevConsole::WARNING = Rgba8(255, 255, 0, 255);
const Rgba8 DevConsole::INFO_MAJOR = Rgba8(0, 255, 255, 255);
const Rgba8 DevConsole::INFO_MINOR = Rgba8(0, 180, 180, 255);
const Rgba8 DevConsole::ECHO = Rgba8(200, 0, 200, 255);
const Rgba8 DevConsole::INPUT_TEXT = Rgba8(200, 200, 255, 255);
const Rgba8 DevConsole::INPUT_INSERTION_POINT = Rgba8(255, 255, 255, 255);

const Rgba8 DevConsole::GAME_MAJOR = Rgba8(255, 80, 0, 255); 
const Rgba8 DevConsole::GAME_MINOR = Rgba8(0, 180, 255, 255);
const Rgba8 DevConsole::GAME_DEBUG = Rgba8(120, 255, 120, 255);

DevConsole::DevConsole(DevConsoleConfig const& config)
	:m_config(config)
{
	m_insertionPointBlinkTimer = new Timer(0.5f, &Clock::GetSystemClock());
	
}

DevConsole::~DevConsole()
{
}

void DevConsole::Startup()
{
	m_fontPath = "Data/Fonts/" + m_config.m_fontName;
	m_defaultFont = m_config.m_renderer->CreateOrGetBitmapFont(m_fontPath.c_str());

	g_theEventSystem->SubscribeEventCallbackFunction("CharInput", DevConsole::Event_CharInput);
	g_theEventSystem->SubscribeEventCallbackFunction("help", DevConsole::Command_Help);
	g_theEventSystem->SubscribeEventCallbackFunction("clear", DevConsole::Command_Clear);

	FireEvent("clear");
	
}

void DevConsole::Shutdown()
{
	

}

void DevConsole::BeginFrame()
{
	m_frameNumber++;
	while (g_theDevConsole->m_insertionPointBlinkTimer->DecrementPeriodIfElapsed()) 
	{
		g_theDevConsole->m_insertionPointVisible = !g_theDevConsole->m_insertionPointVisible;
	}
	
}

void DevConsole::EndFrame()
{
}

void DevConsole::Execute(std::string const& consoleCommandText, bool echoCommand) 
{
	Strings commandLines = SplitStringOnDelimiter(consoleCommandText, '\n');

	for (const std::string& line : commandLines) {
		if (line.empty()) {
			continue;
		}

		Strings parts = SplitStringOnDelimiter(line, ' ');
		if (parts.empty()) {
			continue;
		}

		std::string commandName = parts[0];

		/*
		bool isNoArgCommand = false;
		for (const std::string& noArgCommand : noArgCommands) {
			if (cmpCaseInsensitive()(commandName, noArgCommand)) {
				isNoArgCommand = true;
				break;
			}
		}

		if (isNoArgCommand) {
			if (parts.size() > 1) {
				g_theDevConsole->AddLine(DevConsole::ERROR_COLOR,
					Stringf("Error: Command '%s' does not accept parameters!", commandName.c_str()));
				continue;
			}
		}
		*/

		EventArgs arguments;

		for (int i = 1; i < (int)parts.size(); ++i) 
		{
			Strings keyValue = SplitStringOnDelimiter(parts[i], '=');
			if (keyValue.size() == 2) {
				arguments.SetValue(keyValue[0], keyValue[1]);
			}
		}

		g_theDevConsole->AddLine(DevConsole::ECHO, line);

		if (m_commandHistory.empty() || m_commandHistory.back() != line) 
		{
			m_commandHistory.push_back(line);
		}
		m_historyIndex = (int)m_commandHistory.size();


		std::transform(commandName.begin(), commandName.end(), commandName.begin(),
			[](unsigned char c) { return (char)std::tolower(c); });

		bool success = g_theEventSystem->FireEvent(commandName, arguments);

		if (echoCommand && g_theDevConsole) 
		{
			if (!success) {
				g_theDevConsole->AddLine(DevConsole::ERROR_COLOR, Stringf("Error: Unknown Command '%s'", line.c_str()));
			}
		}
	}
}

void DevConsole::AddLine(Rgba8 const& color, std::string const& text)
{
	DevConsoleLine line;
	line.m_color = color;
	line.m_text = text;
	m_lineVerts.push_back(line);
}

void DevConsole::Render(AABB2 const& bounds, Renderer* rendererOverride) const
{

	if (m_mode == DevConsoleMode::HIDDEN)
	{
		return;
	}

	Renderer* renderer = rendererOverride ? rendererOverride : m_config.m_renderer;
	std::vector<Vertex_PCU> consoleQuadVerts;
	renderer->SetBlendMode(BlendMode::ALPHA);
	AddVertsForAABB2D(consoleQuadVerts, bounds, Rgba8(0, 0, 0, 180));
	renderer->BindTexture(nullptr);
	renderer->DrawVertexArray(consoleQuadVerts);

	if (m_mode == DevConsoleMode::OPEN_FULL)
	{
		BitmapFont* renderFont = renderer->CreateOrGetBitmapFont(m_fontPath.c_str());
		

		Render_OpenFull(bounds, *renderer, *renderFont);
		renderer->BindTexture(nullptr);
	}
}

DevConsoleMode DevConsole::GetMode() const
{
	return m_mode;
}

void DevConsole::SetMode(DevConsoleMode mode)
{
	m_mode = mode;
}

void DevConsole::ToggleOpen()
{
	if (g_theDevConsole)
	{
		if (g_theDevConsole->GetMode() == DevConsoleMode::HIDDEN)
		{
			g_theDevConsole->SetMode(DevConsoleMode::OPEN_FULL);
			m_isOpen = true;
			g_theDevConsole->m_insertionPointBlinkTimer->Start();
			
		}
		else
		{
			g_theDevConsole->SetMode(DevConsoleMode::HIDDEN);
			m_isOpen = false;
			g_theDevConsole->m_insertionPointBlinkTimer->Stop();
			m_historyIndex = (int)m_commandHistory.size();
		}
	}
}

bool DevConsole::IsOpen() const
{
	return m_isOpen;
}

bool DevConsole::Event_KeyPressed(EventArgs& args)
{
	if (!g_theDevConsole)
	{
		return false;
	}
	unsigned char keyCode = (unsigned char)args.GetValue("KeyCode", -1);
	if (keyCode == KEYCODE_ENTER)
	{
		if (g_theDevConsole->m_inputText.empty())
		{
			g_theDevConsole->ToggleOpen();
		}
		else
		{
			g_theDevConsole->Execute(g_theDevConsole->m_inputText);
			g_theDevConsole->m_inputText.clear();
			g_theDevConsole->m_insertionPointPosition = 0;
		}
		
	}
	else if (keyCode == KEYCODE_TILDE || keyCode == KEYCODE_ESC)
	{
		g_theDevConsole->ToggleOpen();
	}
	else if (keyCode == KEYCODE_UPARROW)
	{
		if (!g_theDevConsole->m_commandHistory.empty()
			&& g_theDevConsole->m_historyIndex > 0)
		{
			--g_theDevConsole->m_historyIndex;
			g_theDevConsole->m_inputText = g_theDevConsole->m_commandHistory[g_theDevConsole->m_historyIndex];
			g_theDevConsole->m_insertionPointPosition = (int)g_theDevConsole->m_inputText.size();
		}

	}
	else if (keyCode == KEYCODE_DOWNARROW)
	{
		if (!g_theDevConsole->m_commandHistory.empty() 
			&& g_theDevConsole->m_historyIndex < (int)g_theDevConsole->m_commandHistory.size() - 1)
		{
			++g_theDevConsole->m_historyIndex;
			g_theDevConsole->m_inputText = g_theDevConsole->m_commandHistory[g_theDevConsole->m_historyIndex];
			g_theDevConsole->m_insertionPointPosition = (int)g_theDevConsole->m_inputText.size();
		}
		else
		{
			g_theDevConsole->m_inputText.clear();
			g_theDevConsole->m_historyIndex = (int)g_theDevConsole->m_commandHistory.size();
		}
	}
	else if (keyCode == KEYCODE_LEFTARROW)
	{
		if (g_theDevConsole->m_insertionPointPosition > 0)
		{
			g_theDevConsole->m_insertionPointPosition--;
		}
	}
	else if (keyCode == KEYCODE_RIGHTARROW)
	{
		if (g_theDevConsole->m_insertionPointPosition < static_cast<int>(g_theDevConsole->m_inputText.size()))
		{
			g_theDevConsole->m_insertionPointPosition++;
		}
	}
	else if (keyCode == KEYCODE_HOME)
	{
		g_theDevConsole->m_insertionPointPosition = 0;
	}
	else if (keyCode == KEYCODE_END)
	{
		g_theDevConsole->m_insertionPointPosition = static_cast<int>(g_theDevConsole->m_inputText.size());
	}
	else if (keyCode == KEYCODE_DELETE)
	{
		if (g_theDevConsole->m_insertionPointPosition < static_cast<int>(g_theDevConsole->m_inputText.size()))
		{
			g_theDevConsole->m_inputText.erase(g_theDevConsole->m_insertionPointPosition, 1);
		}
	}
	else if (keyCode == KEYCODE_BACKSPACE)
	{
		if (g_theDevConsole->m_insertionPointPosition > 0)
		{
			g_theDevConsole->m_inputText.erase(g_theDevConsole->m_insertionPointPosition - 1, 1);
			g_theDevConsole->m_insertionPointPosition--;
		}
	}


	return false;
}

bool DevConsole::Event_CharInput(EventArgs& args)
{
	if (!g_theDevConsole)
	{
		return false;
	}

	char ch = (char) args.GetValue("Char", '\0');
	if (ch == '\0') {
		return false;
	}
	else if (ch >= 32 && ch <= 126)
	{
		g_theDevConsole->m_inputText += ch;
		g_theDevConsole->m_insertionPointPosition = (int)g_theDevConsole->m_inputText.size();
		return true;
	}
	
	return false;
}

bool DevConsole::Command_Clear(EventArgs& args)
{
	UNUSED(args);
	g_theDevConsole->m_lineVerts.clear();

	Rgba8 color = Rgba8(0, 200, 50, 255);
	g_theDevConsole->AddLine(color, "Leep's Console v1.0");

	color = DevConsole::INFO_MAJOR;
	g_theDevConsole->AddLine(color, "Type help for a list of commands");

	return true;
}

bool DevConsole::Command_Help(EventArgs& args)
{
	UNUSED(args);
	if (!g_theEventSystem) {
		return false;
	}

	std::vector<std::string> eventNames = g_theEventSystem->GetRegisteredEventNames();

	g_theDevConsole->AddLine(DevConsole::INFO_MAJOR, "Registered Command: ");
	for (const std::string& name : eventNames) 
	{
		g_theDevConsole->AddLine(DevConsole::INFO_MINOR, name);
	}

	return true;
}

void DevConsole::Render_OpenFull(AABB2 const& bounds, Renderer& renderer, BitmapFont& font) const
{
	g_theRenderer->SetDepthMode(DepthMode::DISABLED);
	Vec2 textDimensions = bounds.GetDimensions();
	textDimensions.y = textDimensions.y / m_config.m_linesOnScreen;

	std::vector<Vertex_PCU> textVerts;
	Vec2 prevBoxMin = bounds.m_mins + Vec2(0.f, textDimensions.y);

	for (int lineIndex = 0; lineIndex < (int) m_lineVerts.size(); ++lineIndex)
	{
		const DevConsoleLine& currentLine = m_lineVerts[m_lineVerts.size() - 1 - lineIndex];
		const std::string& lineText = currentLine.m_text;
		const Rgba8& lineColor = currentLine.m_color;

		// Calculate position for each line in the console
		Vec2 linePosition = prevBoxMin + Vec2(0.f, static_cast<float>(lineIndex) * textDimensions.y);
		AABB2 unitBox = AABB2(linePosition, linePosition + textDimensions);

		// Add vertices for the text in the specified box
		unitBox.Translate(Vec2(2.5f, 2.5f));
		font.AddVertsForTextInBox2D(textVerts, lineText, unitBox, textDimensions.y, Rgba8::BLACK, m_config.m_fontAspect, Vec2(0.f, 0.5f));
		unitBox.Translate(Vec2(-2.5f, -2.5f));
		font.AddVertsForTextInBox2D(textVerts, lineText, unitBox, textDimensions.y, lineColor, m_config.m_fontAspect, Vec2(0.f, 0.5f));

		renderer.BindTexture(&font.GetTexture());
		renderer.DrawVertexArray(textVerts);

		// Clear vertices after drawing
		textVerts.clear();
	}


	Vec2 linePosition = bounds.m_mins;
	AABB2 unitBox = AABB2(linePosition, linePosition + textDimensions);

	if (m_inputText.size() > 0)
	{
		unitBox.Translate(Vec2(2.5f, 2.5f));
		font.AddVertsForTextInBox2D(textVerts, g_theDevConsole->m_inputText, unitBox, textDimensions.y, Rgba8::BLACK, m_config.m_fontAspect, Vec2(0.f, 0.5f));

		unitBox.Translate(Vec2(-2.5f, -2.5f));
		font.AddVertsForTextInBox2D(textVerts, g_theDevConsole->m_inputText, unitBox, textDimensions.y, DevConsole::INPUT_TEXT, m_config.m_fontAspect, Vec2(0.f, 0.5f));

		renderer.BindTexture(&font.GetTexture());
		renderer.DrawVertexArray(textVerts);
		textVerts.clear();
	}
	

	if (g_theDevConsole->m_insertionPointVisible)
	{
		float cursorX = font.GetTextWidth(textDimensions.y, m_inputText.substr(0, m_insertionPointPosition), m_config.m_fontAspect);
		Vec2 cursorPosition = bounds.m_mins + Vec2(cursorX, 0.f);

		AABB2 cursorBox = AABB2(cursorPosition, cursorPosition + Vec2(2.5f, textDimensions.y));

		AddVertsForAABB2D(textVerts, cursorBox, DevConsole::INPUT_INSERTION_POINT);

		renderer.BindTexture(nullptr);
		renderer.DrawVertexArray(textVerts);
	}
}