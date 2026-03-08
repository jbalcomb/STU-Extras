// Tests for the undo/redo stack.
// Powered by Claude.

#include <gtest/gtest.h>
#include "editor/UndoStack.hpp"

using namespace mom;

TEST(UndoStack, InitiallyEmpty) {
    UndoStack stack;
    EXPECT_FALSE(stack.can_undo());
    EXPECT_FALSE(stack.can_redo());
    EXPECT_EQ(stack.undo_depth(), 0);
    EXPECT_EQ(stack.redo_depth(), 0);
}

TEST(UndoStack, ExecuteAddsToStack) {
    UndoStack stack;
    int value = 0;

    stack.execute(std::make_unique<LambdaCommand>(
        [&]() { value = 42; },
        [&]() { value = 0; }
    ));

    EXPECT_EQ(value, 42);
    EXPECT_TRUE(stack.can_undo());
    EXPECT_FALSE(stack.can_redo());
    EXPECT_EQ(stack.undo_depth(), 1);
}

TEST(UndoStack, UndoRevertsCommand) {
    UndoStack stack;
    int value = 0;

    stack.execute(std::make_unique<LambdaCommand>(
        [&]() { value = 42; },
        [&]() { value = 0; }
    ));
    EXPECT_EQ(value, 42);

    EXPECT_TRUE(stack.undo());
    EXPECT_EQ(value, 0);
    EXPECT_FALSE(stack.can_undo());
    EXPECT_TRUE(stack.can_redo());
}

TEST(UndoStack, RedoReappliesCommand) {
    UndoStack stack;
    int value = 0;

    stack.execute(std::make_unique<LambdaCommand>(
        [&]() { value = 42; },
        [&]() { value = 0; }
    ));
    stack.undo();
    EXPECT_EQ(value, 0);

    EXPECT_TRUE(stack.redo());
    EXPECT_EQ(value, 42);
    EXPECT_TRUE(stack.can_undo());
    EXPECT_FALSE(stack.can_redo());
}

TEST(UndoStack, MultipleUndoRedo) {
    UndoStack stack;
    int value = 0;

    stack.execute(std::make_unique<LambdaCommand>(
        [&]() { value = 1; },
        [&]() { value = 0; }
    ));
    stack.execute(std::make_unique<LambdaCommand>(
        [&]() { value = 2; },
        [&]() { value = 1; }
    ));
    stack.execute(std::make_unique<LambdaCommand>(
        [&]() { value = 3; },
        [&]() { value = 2; }
    ));
    EXPECT_EQ(value, 3);
    EXPECT_EQ(stack.undo_depth(), 3);

    stack.undo();
    EXPECT_EQ(value, 2);

    stack.undo();
    EXPECT_EQ(value, 1);

    stack.redo();
    EXPECT_EQ(value, 2);

    stack.redo();
    EXPECT_EQ(value, 3);
}

TEST(UndoStack, NewCommandClearsRedoHistory) {
    UndoStack stack;
    int value = 0;

    stack.execute(std::make_unique<LambdaCommand>(
        [&]() { value = 1; },
        [&]() { value = 0; }
    ));
    stack.execute(std::make_unique<LambdaCommand>(
        [&]() { value = 2; },
        [&]() { value = 1; }
    ));

    // Undo one, then execute a new command
    stack.undo();
    EXPECT_EQ(value, 1);
    EXPECT_TRUE(stack.can_redo());

    stack.execute(std::make_unique<LambdaCommand>(
        [&]() { value = 99; },
        [&]() { value = 1; }
    ));
    EXPECT_EQ(value, 99);
    EXPECT_FALSE(stack.can_redo()); // Redo history cleared
    EXPECT_EQ(stack.undo_depth(), 2);
}

TEST(UndoStack, UndoOnEmptyReturnsFalse) {
    UndoStack stack;
    EXPECT_FALSE(stack.undo());
}

TEST(UndoStack, RedoOnEmptyReturnsFalse) {
    UndoStack stack;
    EXPECT_FALSE(stack.redo());
}

TEST(UndoStack, MaxDepthEvictsOldest) {
    UndoStack stack;
    int values[UndoStack::MAX_DEPTH + 10] = {};

    // Push more than MAX_DEPTH commands
    for (int i = 0; i < UndoStack::MAX_DEPTH + 5; ++i) {
        int idx = i;
        stack.execute(std::make_unique<LambdaCommand>(
            [&values, idx]() { values[idx] = 1; },
            [&values, idx]() { values[idx] = 0; }
        ));
    }

    // Should be capped at MAX_DEPTH
    EXPECT_EQ(stack.undo_depth(), UndoStack::MAX_DEPTH);

    // Undo all should only undo MAX_DEPTH commands
    int undo_count = 0;
    while (stack.undo()) ++undo_count;
    EXPECT_EQ(undo_count, UndoStack::MAX_DEPTH);
}

TEST(UndoStack, ClearRemovesAll) {
    UndoStack stack;
    int value = 0;

    stack.execute(std::make_unique<LambdaCommand>(
        [&]() { value = 1; },
        [&]() { value = 0; }
    ));
    stack.undo();
    // Now has redo history

    stack.clear();
    EXPECT_FALSE(stack.can_undo());
    EXPECT_FALSE(stack.can_redo());
    EXPECT_EQ(stack.undo_depth(), 0);
    EXPECT_EQ(stack.redo_depth(), 0);
}

TEST(UndoStack, ComplexSequence) {
    UndoStack stack;
    std::string log;

    stack.execute(std::make_unique<LambdaCommand>(
        [&]() { log += "A"; },
        [&]() { log.pop_back(); }
    ));
    stack.execute(std::make_unique<LambdaCommand>(
        [&]() { log += "B"; },
        [&]() { log.pop_back(); }
    ));
    EXPECT_EQ(log, "AB");

    stack.undo(); // remove B
    EXPECT_EQ(log, "A");

    stack.execute(std::make_unique<LambdaCommand>(
        [&]() { log += "C"; },
        [&]() { log.pop_back(); }
    ));
    EXPECT_EQ(log, "AC");
    EXPECT_FALSE(stack.can_redo()); // B's redo is gone

    stack.undo(); // remove C
    EXPECT_EQ(log, "A");
    stack.undo(); // remove A
    EXPECT_EQ(log, "");

    stack.redo(); // reapply A
    EXPECT_EQ(log, "A");
    stack.redo(); // reapply C
    EXPECT_EQ(log, "AC");
}
