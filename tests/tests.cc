#ifndef CATCH_CONFIG_MAIN
#  define CATCH_CONFIG_MAIN
#endif

#include "atm.hpp"
#include "catch.hpp"

/////////////////////////////////////////////////////////////////////////////////////////////
//                             Helper Definitions //
/////////////////////////////////////////////////////////////////////////////////////////////

bool CompareFiles(const std::string& p1, const std::string& p2) {
  std::ifstream f1(p1);
  std::ifstream f2(p2);

  if (f1.fail() || f2.fail()) {
    return false;  // file problem
  }

  std::string f1_read;
  std::string f2_read;
  while (f1.good() || f2.good()) {
    f1 >> f1_read;
    f2 >> f2_read;
    if (f1_read != f2_read || (f1.good() && !f2.good()) ||
        (!f1.good() && f2.good()))
      return false;
  }
  return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Test Cases
/////////////////////////////////////////////////////////////////////////////////////////////

TEST_CASE("Example: Create a new account", "[ex-1]") {
  Atm atm;
  atm.RegisterAccount(12345678, 1234, "Sam Sepiol", 300.30);
  auto accounts = atm.GetAccounts();
  REQUIRE(accounts.contains({12345678, 1234}));
  REQUIRE(accounts.size() == 1);

  Account sam_account = accounts[{12345678, 1234}];
  REQUIRE(sam_account.owner_name == "Sam Sepiol");
  REQUIRE(sam_account.balance == 300.30);

  auto transactions = atm.GetTransactions();
  REQUIRE(accounts.contains({12345678, 1234}));
  REQUIRE(accounts.size() == 1);
  std::vector<std::string> empty;
  REQUIRE(transactions[{12345678, 1234}] == empty);
}

TEST_CASE("Example: Simple widthdraw", "[ex-2]") {
  Atm atm;
  atm.RegisterAccount(12345678, 1234, "Sam Sepiol", 300.30);
  atm.WithdrawCash(12345678, 1234, 20);
  auto accounts = atm.GetAccounts();
  Account sam_account = accounts[{12345678, 1234}];

  REQUIRE(sam_account.balance == 280.30);
}

TEST_CASE("Example: Print Prompt Ledger", "[ex-3]") {
  Atm atm;
  atm.RegisterAccount(12345678, 1234, "Sam Sepiol", 300.30);
  auto& transactions = atm.GetTransactions();
  transactions[{12345678, 1234}].push_back(
      "Withdrawal - Amount: $200.40, Updated Balance: $99.90");
  transactions[{12345678, 1234}].push_back(
      "Deposit - Amount: $40000.00, Updated Balance: $40099.90");
  transactions[{12345678, 1234}].push_back(
      "Deposit - Amount: $32000.00, Updated Balance: $72099.90");
  atm.PrintLedger("./prompt.txt", 12345678, 1234);
  REQUIRE(CompareFiles("./ex-1.txt", "./prompt.txt"));
}

// my_test_cases

TEST_CASE("Register duplicate account throws exception", "[ex-dup]") {
  Atm atm;
  atm.RegisterAccount(11111111, 2222, "Alice", 100.0);
  REQUIRE_THROWS_AS(atm.RegisterAccount(11111111, 2222, "Alice", 200.0),
                    std::invalid_argument);
}

TEST_CASE("Withdraw more than balance throws exception", "[ex-overdraft]") {
  Atm atm;
  atm.RegisterAccount(22222222, 3333, "Bob", 50.0);
  REQUIRE_THROWS_AS(atm.WithdrawCash(22222222, 3333, 100.0),
                    std::runtime_error);
}

TEST_CASE("Negative withdrawal and deposit throw exception", "[ex-negative]") {
  Atm atm;
  atm.RegisterAccount(33333333, 4444, "Carol", 500.0);

  REQUIRE_THROWS_AS(atm.WithdrawCash(33333333, 4444, -50.0),
                    std::invalid_argument);
  REQUIRE_THROWS_AS(atm.DepositCash(33333333, 4444, -25.0),
                    std::invalid_argument);
}

TEST_CASE("CheckBalance or Withdraw on non-existent account throws",
          "[ex-invalid-account]") {
  Atm atm;
  REQUIRE_THROWS_AS(atm.CheckBalance(99999999, 1234), std::invalid_argument);
  REQUIRE_THROWS_AS(atm.WithdrawCash(99999999, 1234, 10.0),
                    std::invalid_argument);
}

TEST_CASE("Zero deposit and withdrawal should be allowed", "[ex-zero]") {
  Atm atm;
  atm.RegisterAccount(44444444, 5555, "Dave", 100.0);

  // Zero deposit should not throw, balance unchanged
  atm.DepositCash(44444444, 5555, 0.0);
  REQUIRE(atm.CheckBalance(44444444, 5555) == Approx(100.0));

  // Zero withdrawal should not throw, balance unchanged
  atm.WithdrawCash(44444444, 5555, 0.0);
  REQUIRE(atm.CheckBalance(44444444, 5555) == Approx(100.0));
}

TEST_CASE("Same card number but different PINs are different accounts",
          "[ex-multi-pin]") {
  Atm atm;
  atm.RegisterAccount(55555555, 1111, "Eve", 50.0);
  atm.RegisterAccount(55555555, 2222, "Frank", 75.0);

  auto accounts = atm.GetAccounts();
  REQUIRE(accounts.size() == 2);
  REQUIRE(accounts[{55555555, 1111}].owner_name == "Eve");
  REQUIRE(accounts[{55555555, 2222}].owner_name == "Frank");
}

TEST_CASE("Print ledger for account with no transactions",
          "[ex-empty-ledger]") {
  Atm atm;
  atm.RegisterAccount(66666666, 3333, "Grace", 200.0);
  atm.PrintLedger("empty-ledger.txt", 66666666, 3333);

  std::ifstream f("empty-ledger.txt");
  REQUIRE(f.good());
}

TEST_CASE("Large deposit should update balance correctly", "[ex-large]") {
  Atm atm;
  atm.RegisterAccount(77777777, 4444, "Henry", 10.0);
  atm.DepositCash(77777777, 4444, 1e9);
  // test
  REQUIRE(atm.CheckBalance(77777777, 4444) == Approx(1000000010.0));
}
