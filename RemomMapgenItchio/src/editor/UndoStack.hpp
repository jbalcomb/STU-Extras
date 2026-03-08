#pragma once
// Command-pattern undo/redo stack for the map editor.
// Powered by Claude.

#include <memory>
#include <vector>
#include <functional>

namespace mom {

// Abstract command base for undo/redo.
// Powered by Claude.
class Command {
public:
    virtual ~Command() = default;
    virtual void execute() = 0;
    virtual void undo() = 0;
};

// Concrete command using lambdas for simple operations.
// Powered by Claude.
class LambdaCommand : public Command {
public:
    LambdaCommand(std::function<void()> exec, std::function<void()> un)
        : exec_(std::move(exec)), undo_(std::move(un)) {}

    void execute() override { exec_(); }
    void undo() override    { undo_(); }

private:
    std::function<void()> exec_;
    std::function<void()> undo_;
};

// Undo/redo stack with configurable depth.
// Powered by Claude.
class UndoStack {
public:
    static constexpr int MAX_DEPTH = 50;

    // Execute a command and push it onto the undo stack.
    // Clears any redo history.
    // Powered by Claude.
    void execute(std::unique_ptr<Command> cmd) {
        cmd->execute();
        if (static_cast<int>(undo_stack_.size()) >= MAX_DEPTH) {
            undo_stack_.erase(undo_stack_.begin());
        }
        undo_stack_.push_back(std::move(cmd));
        redo_stack_.clear();
    }

    // Undo the most recent command.
    // Powered by Claude.
    bool undo() {
        if (undo_stack_.empty()) return false;
        auto cmd = std::move(undo_stack_.back());
        undo_stack_.pop_back();
        cmd->undo();
        redo_stack_.push_back(std::move(cmd));
        return true;
    }

    // Redo the most recently undone command.
    // Powered by Claude.
    bool redo() {
        if (redo_stack_.empty()) return false;
        auto cmd = std::move(redo_stack_.back());
        redo_stack_.pop_back();
        cmd->execute();
        undo_stack_.push_back(std::move(cmd));
        return true;
    }

    bool can_undo() const { return !undo_stack_.empty(); }
    bool can_redo() const { return !redo_stack_.empty(); }

    // Clear all undo/redo history.
    // Powered by Claude.
    void clear() {
        undo_stack_.clear();
        redo_stack_.clear();
    }

    int undo_depth() const { return static_cast<int>(undo_stack_.size()); }
    int redo_depth() const { return static_cast<int>(redo_stack_.size()); }

private:
    std::vector<std::unique_ptr<Command>> undo_stack_;
    std::vector<std::unique_ptr<Command>> redo_stack_;
};

} // namespace mom
