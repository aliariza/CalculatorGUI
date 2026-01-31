#include "calculator.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <iomanip>
#include <limits>
#include <sstream>
#include <string>

Calculator::Calculator() {
    clearAll();
}

// ---------------------------
// Helpers (file-local)
// ---------------------------

static void stripChars(std::string& s, char ch) {
    s.erase(std::remove(s.begin(), s.end(), ch), s.end());
}

// Converts an INTERNAL string like "1234.50" into Turkish DISPLAY string "1.234,5"
static std::string formatTR_fromInternal(std::string s) {
    // Scientific notation: leave as-is
    if (s.find('e') != std::string::npos || s.find('E') != std::string::npos)
        return s;

    // Sign
    std::string sign;
    if (!s.empty() && (s[0] == '-' || s[0] == '+')) {
        sign = s.substr(0, 1);
        s.erase(0, 1);
    }

    // Split on internal decimal '.'
    std::string intPart = s;
    std::string fracPart;

    auto dotPos = s.find('.');
    if (dotPos != std::string::npos) {
        intPart = s.substr(0, dotPos);
        fracPart = s.substr(dotPos + 1);
    }

    // Clean any old separators (just in case)
    stripChars(intPart, ',');
    stripChars(intPart, '.');
    stripChars(fracPart, ',');
    stripChars(fracPart, '.');

    // Insert '.' every 3 digits in integer part
    for (int i = (int)intPart.size() - 3; i > 0; i -= 3) {
        intPart.insert(i, ".");
    }

    // Use ',' as decimal separator on display
    if (!fracPart.empty()) {
        return sign + intPart + "," + fracPart;
    }
    return sign + intPart;
}

// Build an INTERNAL number string (no thousand separators, decimal is '.')
static std::string toInternalString(long double v) {
    // If close to integer, output integer
    long double rounded = std::round(v);
    if (std::fabsl(v - rounded) < 1e-12L) {
        std::ostringstream oss;
        if (rounded >= (long double)std::numeric_limits<long long>::min() &&
            rounded <= (long double)std::numeric_limits<long long>::max()) {
            oss << (long long)rounded;
        } else {
            oss << std::setprecision(10) << std::scientific << (double)v;
        }
        return oss.str();
    }

    // Otherwise output fixed precision then trim zeros
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(4) << (double)v;
    std::string s = oss.str();

    if (s.find('.') != std::string::npos) {
        while (!s.empty() && s.back() == '0') s.pop_back();
        if (!s.empty() && s.back() == '.') s.pop_back();
    }
    return s;
}

// ---------------------------
// Core input handling
// ---------------------------

bool Calculator::press(char key) {
    // Memory / percent should work even in error state
    if (key == 'X') { memoryClear(); return true; } // MC
    if (key == 'R') { memoryRecall(); return true; } // MR
    if (key == 'M') { memoryAdd(); return true; } // M+
    if (key == '%') { percent(); return true; }

    if (key == ' ' || key == '\n' || key == '\r' || key == '\t') return false;

    // If error, only allow clear
    if (error_) {
        if (key == 'c' || key == 'C') {
            clearAll();
            return true;
        }
        return false;
    }

    // Digits
    if (key >= '0' && key <= '9') {
        if (justEvaluated_) {
            // Start new entry after '=' if user types a digit
            acc_.reset();
            op_.reset();
            entry_.clear();
            justEvaluated_ = false;
        }
        return appendDigit(key);
    }

    // Clear
    if (key == 'c' || key == 'C') {
        clearAll();
        return true;
    }

    // Backspace
    if (key == 'b' || key == 'B') {
        backspace();
        return true;
    }

    // Decimal point (accept both '.' and ',' from keyboard)
    if (key == '.' || key == ',') {
        if (justEvaluated_) {
            acc_.reset();
            op_.reset();
            entry_.clear();
            justEvaluated_ = false;
        }

        if (entry_.empty())
            entry_ = "0.";
        else if (entry_.find('.') == std::string::npos)
            entry_ += '.';

        return true;
    }

    // Operators
    if (key == '+' || key == '-' || key == '*' || key == '/') {
        // If we just evaluated, continue with result as accumulator
        justEvaluated_ = false;

        // If no accumulator yet, move entry to accumulator (or default 0)
        if (!acc_.has_value()) {
            if (!entry_.empty()) acc_ = entryValue();
            else acc_ = 0.0L;
        } else {
            // If there's a pending op and an entry, apply it first (chained operations)
            if (op_.has_value() && !entry_.empty()) {
                if (!applyPendingOp(entryValue())) return true;
            }
        }

        // Set new pending operator
        op_ = key;
        entry_.clear();
        return true;
    }

    // Equals
    if (key == '=') {
        if (!acc_.has_value()) {
            // Nothing stored; '=' just keeps current entry (or 0)
            if (entry_.empty()) entry_ = "0";
            justEvaluated_ = true;
            return true;
        }

        if (op_.has_value()) {
            if (entry_.empty()) {
                setError("Enter a number before '='");
                return true;
            }
            if (!applyPendingOp(entryValue())) return true;

            // Store result internally (NOT Turkish formatted text)
            setEntryFromNumber(*acc_);
            op_.reset();
            justEvaluated_ = true;
            return true;
        }

        // If no op pending, show accumulator as entry
        setEntryFromNumber(*acc_);
        justEvaluated_ = true;
        return true;
    }

    return false; // ignored key
}

// ---------------------------
// UI / status
// ---------------------------

std::string Calculator::display() const {
    if (error_) return "ERROR";

    // entry_ is internal -> format for Turkish display
    if (!entry_.empty()) {
        return formatTR_fromInternal(entry_);
    }

    if (acc_.has_value()) return formatNumber(*acc_);
    return "0";
}

std::string Calculator::statusLine() const {
    if (error_) return errorMsg_;

    std::ostringstream oss;
    if (acc_.has_value()) {
        oss << formatNumber(*acc_);
        if (op_.has_value()) oss << " " << *op_;
    } else {
        oss << "Ready";
    }
    return oss.str();
}
bool Calculator::hasMemory() const {
    return mem_.has_value();
}
bool Calculator::hasError() const { return error_; }
std::string Calculator::errorMessage() const { return errorMsg_; }

// ---------------------------
// State helpers
// ---------------------------

void Calculator::clearAll() {
    entry_.clear();
    acc_.reset();
    op_.reset();
    justEvaluated_ = false;
    clearError();
    // IMPORTANT: memory (mem_) is NOT cleared here (only by MC)
}

void Calculator::setError(const std::string& msg) {
    error_ = true;
    errorMsg_ = msg;
}

void Calculator::clearError() {
    error_ = false;
    errorMsg_.clear();
}

bool Calculator::appendDigit(char d) {
    // Count only digits (ignore '.')
    int digits = 0;
    for (char c : entry_) if (std::isdigit((unsigned char)c)) digits++;

    if (digits >= kMaxDigits) {
        setError("Max 10 digits");
        return true;
    }

    // Avoid leading zeros like "0002" -> keep "0" then replace
    if (entry_ == "0") {
        entry_ = std::string(1, d);
        return true;
    }

    // If entry is "0." allow "0.5" etc.
    entry_.push_back(d);
    return true;
}

void Calculator::backspace() {
    if (!entry_.empty()) {
        entry_.pop_back();
        if (entry_.empty()) justEvaluated_ = false;
    }
}

long double Calculator::entryValue() const {
    // entry_ must be INTERNAL format only (digits + optional '.')
    return std::stold(entry_);
}

void Calculator::setEntryFromNumber(long double v) {
    // Store INTERNAL string, not Turkish display formatting
    entry_ = toInternalString(v);
}

bool Calculator::applyPendingOp(long double rhs) {
    if (!acc_.has_value() || !op_.has_value()) {
        acc_ = rhs;
        return true;
    }

    const char op = *op_;
    long double lhs = *acc_;
    long double out = 0.0L;

    if (op == '+') out = lhs + rhs;
    else if (op == '-') out = lhs - rhs;
    else if (op == '*') out = lhs * rhs;
    else if (op == '/') {
        if (rhs == 0.0L) {
            setError("Division by zero");
            return false;
        }
        out = lhs / rhs;
    }

    if (!std::isfinite((double)out)) {
        setError("Overflow/invalid");
        return false;
    }

    acc_ = out;
    return true;
}

// ---------------------------
// Display formatting
// ---------------------------

std::string Calculator::formatNumber(long double v) {
    // Convert to INTERNAL string first, then Turkish-display format it
    const std::string internal = toInternalString(v);
    return formatTR_fromInternal(internal);
}

// ---------------------------
// Memory + percent
// ---------------------------

void Calculator::memoryClear() {
    mem_.reset();
}

void Calculator::memoryRecall() {
    if (!mem_.has_value()) return;

    // Recall into INTERNAL entry
    setEntryFromNumber(*mem_);
    justEvaluated_ = false;
    clearError();
}

void Calculator::memoryAdd() {
    long double v = 0.0L;

    if (!entry_.empty()) v = entryValue();
    else if (acc_.has_value()) v = *acc_;
    else v = 0.0L;

    if (!mem_.has_value()) mem_ = 0.0L;
    *mem_ += v;
}

void Calculator::percent() {
    if (entry_.empty()) {
        if (acc_.has_value()) {
            long double v = *acc_ / 100.0L;
            setEntryFromNumber(v); // INTERNAL
        }
        return;
    }

    long double e = entryValue();

    if (acc_.has_value() && op_.has_value()) {
        e = (*acc_) * (e / 100.0L);
    } else {
        e = e / 100.0L;
    }

    setEntryFromNumber(e); // INTERNAL
    justEvaluated_ = false;
}
