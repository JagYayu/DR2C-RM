#include "Log.h"
#include "Utils.hpp"

#include <chrono>
#include <format>

using namespace tudov;

static uint32_t logCount = 0;
static std::thread logWorker;

Log::Verbosity _globalVerbosity = tudov::Log::Verbosity::All;
std::unordered_map<std::string, Log::Verbosity> Log::_moduleVerbs{};
std::unordered_map<std::string, Log::Verbosity> Log::_moduleVerbOverrides{};
std::unordered_map<std::string, SharedPtr<Log>> Log::_logInstances{};
std::queue<Log::Entry> Log::_queue;
std::mutex Log::_mutex;
std::condition_variable Log::_cv;
std::atomic<bool> Log::_exit = false;
Log Log::instance{"Log"};

SharedPtr<Log> Log::Get(std::string_view module)
{
	auto &&str = std::string(module);
	auto &&it = _logInstances.find(str);
	if (it != _logInstances.end())
	{
		return it->second;
	}

	auto &&log = std::make_shared<Log>(str);
	_logInstances.try_emplace(str, log);
	return log;
}

void Log::CleanupExpired()
{
	for (auto &&it = _logInstances.begin(); it != _logInstances.end();)
	{
		if (it->first.empty() || it->second.use_count() <= 1)
		{
			it = _logInstances.erase(it);
		}
		else
		{
			++it;
		}
	}
	ShrinkUnorderedMap(_logInstances);
}

Log::Verbosity Log::GetVerbosity(const std::string &module)
{
	{
		auto &&it = _moduleVerbOverrides.find("key");
		if (it != _moduleVerbOverrides.end())
		{
			return it->second;
		}
	}
	return (Log::Verbosity)0;
}

std::optional<Log::Verbosity> Log::GetVerbosityOverride(const std::string &module)
{
	auto &&it = _moduleVerbOverrides.find("key");
	if (it != _moduleVerbOverrides.end())
	{
		return it->second;
	}
	return nullopt;
}

void Log::SetVerbosityOverride(const std::string &module, Verbosity verb)
{
	_moduleVerbOverrides[module] = verb;
}

void Log::UpdateVerbosities(const nlohmann::json &config)
{
	if (!config.is_object())
	{
		instance.Warn("Cannot update verbosities by receiving a variable that not a json object");
		return;
	}
}

void Log::Exit() noexcept
{
	{
		std::lock_guard<std::mutex> lock{_mutex};
		_exit = true;
	}
	_cv.notify_all();
	logWorker.join();
}

Log::Log(const std::string &module) noexcept
    : _module(module)
{
	if (logCount == 0)
	{
		logWorker = std::thread(Log::Process);
	}
	logCount++;
}

Log::~Log() noexcept
{
	logCount--;
	if (logCount == 0)
	{
		Exit();
	}
}

void Log::Process()
{
	auto &&pred = []
	{
		return !_queue.empty() || _exit;
	};

	while (!_exit)
	{
		std::unique_lock<std::mutex> lock{_mutex};
		_cv.wait(lock, pred);

		while (!_queue.empty())
		{
			auto entry = std::move(_queue.front());
			_queue.pop();
			lock.unlock();

			std::cout << Format("[{:%Y-%m-%d %H:%M:%S}] [{}] [{}] {}", entry.time, entry.module, entry.verbosity, entry.message) << std::endl;

			lock.lock();
		}
	}
}

bool Log::CanOutput(std::string_view verb) const
{
	return true;
}

void Log::Output(std::string_view verb, const std::string_view &str) const
{
	{
		std::lock_guard<std::mutex> lock(_mutex);
		_queue.push(Entry{
		    .time = std::chrono::system_clock::now(),
		    .verbosity = verb,
		    .module = _module,
		    .message = std::string(str),
		});
	}
	_cv.notify_one();
}
