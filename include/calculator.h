#ifndef CALCULATOR_H
#define CALCULATOR_H

#include <string>
#include <optional>

class Calculator {
public:
    Calculator();

    // Key inputs: '0'..'9', '+', '-', '*', '/', '=', 'c', 'b'
    // Returns false if key ignored.
    bool press(char key);

    // For UI
    std::string display() const;          // what to show on screen (current entry or result)
    std::string statusLine() const;       // small status text (e.g., "12 +")
    bool hasError() const;
    bool hasMemory() const;
    std::string errorMessage() const;

    // Memory + percent
    void memoryClear();   // MC
    void memoryRecall();  // MR
    void memoryAdd();     // M+
    void percent();       // %

private:
    // State
    std::string entry_;                   // current number being typed (digits only, maybe leading '-')
    std::optional<long double> acc_;      // accumulator / stored value
    std::optional<char> op_;              // pending operator
    bool justEvaluated_ = false;

    bool error_ = false;
    std::string errorMsg_;

    static constexpr int kMaxDigits = 10;

    // Helpers
    void clearAll();
    void setError(const std::string& msg);
    void clearError();

    bool appendDigit(char d);
    void backspace();
    long double entryValue() const;
    void setEntryFromNumber(long double v);

    bool applyPendingOp(long double rhs); // apply acc_ (op_) rhs -> acc_
    static std::string formatNumber(long double v);
    std::optional<long double> mem_;  // memory register

};

#endif
