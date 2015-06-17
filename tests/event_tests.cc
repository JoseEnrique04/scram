#include "event.h"

#include <boost/shared_ptr.hpp>
#include <gtest/gtest.h>

#include "cycle.h"
#include "error.h"

using namespace scram;

typedef boost::shared_ptr<Event> EventPtr;
typedef boost::shared_ptr<Gate> GatePtr;
typedef boost::shared_ptr<Formula> FormulaPtr;
typedef boost::shared_ptr<PrimaryEvent> PrimaryEventPtr;
typedef boost::shared_ptr<HouseEvent> HouseEventPtr;
typedef boost::shared_ptr<BasicEvent> BasicEventPtr;

// Test for Event base class.
TEST(EventTest, Id) {
  EventPtr event(new Event("event_name"));
  EXPECT_EQ(event->id(), "event_name");
}

TEST(FormulaTest, VoteNumber) {
  FormulaPtr top(new Formula("and"));
  EXPECT_EQ("and", top->type());
  // Setting a vote number for non-Vote fromula is an error.
  EXPECT_THROW(top->vote_number(2), LogicError);
  // Resetting to VOTE formula.
  top = FormulaPtr(new Formula("atleast"));
  EXPECT_EQ("atleast", top->type());
  // No vote number.
  EXPECT_THROW(top->vote_number(), LogicError);
  // Illegal vote number.
  EXPECT_THROW(top->vote_number(-2), InvalidArgument);
  // Legal vote number.
  EXPECT_NO_THROW(top->vote_number(2));
  // Trying to reset the vote number.
  EXPECT_THROW(top->vote_number(2), LogicError);
  // Requesting the vote number should succeed.
  ASSERT_NO_THROW(top->vote_number());
  EXPECT_EQ(2, top->vote_number());
}

TEST(FormulaTest, Arguments) {
  FormulaPtr top(new Formula("and"));
  std::map<std::string, EventPtr> children;
  EventPtr first_child(new Event("first"));
  EventPtr second_child(new Event("second"));
  // Request for children when there are no children is an error.
  EXPECT_THROW(top->event_args(), LogicError);
  // Adding first child.
  EXPECT_NO_THROW(top->AddArgument(first_child));
  // Re-adding a child must cause an error.
  EXPECT_THROW(top->AddArgument(first_child), LogicError);
  // Check the contents of the children container.
  children.insert(std::make_pair(first_child->id(), first_child));
  EXPECT_EQ(children, top->event_args());
  // Adding another child.
  EXPECT_NO_THROW(top->AddArgument(second_child));
  children.insert(std::make_pair(second_child->id(), second_child));
  EXPECT_EQ(children, top->event_args());
}

TEST(GateTest, Cycle) {
  GatePtr top(new Gate("Top"));
  top->name("Top");
  FormulaPtr formula_one(new Formula("not"));
  top->formula(formula_one);
  GatePtr middle(new Gate("Middle"));
  middle->name("Middle");
  FormulaPtr formula_two(new Formula("not"));
  middle->formula(formula_two);
  GatePtr bottom(new Gate("Bottom"));
  bottom->name("Bottom");
  FormulaPtr formula_three(new Formula("not"));
  bottom->formula(formula_three);
  formula_one->AddArgument(middle);
  formula_two->AddArgument(bottom);
  formula_three->AddArgument(top);  // Looping here.
  std::vector<std::string> cycle;
  bool ret = cycle::DetectCycle<Gate, Formula>(&*top, &cycle);
  EXPECT_TRUE(ret);
  std::vector<std::string> print_cycle;
  print_cycle.push_back("Top");
  print_cycle.push_back("Bottom");
  print_cycle.push_back("Middle");
  print_cycle.push_back("Top");
  EXPECT_EQ(print_cycle, cycle);
  EXPECT_EQ("Top->Middle->Bottom->Top", cycle::PrintCycle(cycle));
}

// Test gate type validation.
TEST(FormulaTest, Validate) {
  FormulaPtr top(new Formula("and"));
  BasicEventPtr A(new BasicEvent("a"));
  BasicEventPtr B(new BasicEvent("b"));
  BasicEventPtr C(new BasicEvent("c"));

  // AND Formula tests.
  EXPECT_THROW(top->Validate(), ValidationError);
  top->AddArgument(A);
  EXPECT_THROW(top->Validate(), ValidationError);
  top->AddArgument(B);
  EXPECT_NO_THROW(top->Validate());
  top->AddArgument(C);
  EXPECT_NO_THROW(top->Validate());

  // OR Formula tests.
  top = FormulaPtr(new Formula("or"));
  EXPECT_THROW(top->Validate(), ValidationError);
  top->AddArgument(A);
  EXPECT_THROW(top->Validate(), ValidationError);
  top->AddArgument(B);
  EXPECT_NO_THROW(top->Validate());
  top->AddArgument(C);
  EXPECT_NO_THROW(top->Validate());

  // NOT Formula tests.
  top = FormulaPtr(new Formula("not"));
  EXPECT_THROW(top->Validate(), ValidationError);
  top->AddArgument(A);
  EXPECT_NO_THROW(top->Validate());
  top->AddArgument(B);
  EXPECT_THROW(top->Validate(), ValidationError);

  // NULL Formula tests.
  top = FormulaPtr(new Formula("null"));
  EXPECT_THROW(top->Validate(), ValidationError);
  top->AddArgument(A);
  EXPECT_NO_THROW(top->Validate());
  top->AddArgument(B);
  EXPECT_THROW(top->Validate(), ValidationError);

  // NOR Formula tests.
  top = FormulaPtr(new Formula("nor"));
  EXPECT_THROW(top->Validate(), ValidationError);
  top->AddArgument(A);
  EXPECT_THROW(top->Validate(), ValidationError);
  top->AddArgument(B);
  EXPECT_NO_THROW(top->Validate());
  top->AddArgument(C);
  EXPECT_NO_THROW(top->Validate());

  // NAND Formula tests.
  top = FormulaPtr(new Formula("nand"));
  EXPECT_THROW(top->Validate(), ValidationError);
  top->AddArgument(A);
  EXPECT_THROW(top->Validate(), ValidationError);
  top->AddArgument(B);
  EXPECT_NO_THROW(top->Validate());
  top->AddArgument(C);
  EXPECT_NO_THROW(top->Validate());

  // XOR Formula tests.
  top = FormulaPtr(new Formula("xor"));
  EXPECT_THROW(top->Validate(), ValidationError);
  top->AddArgument(A);
  EXPECT_THROW(top->Validate(), ValidationError);
  top->AddArgument(B);
  EXPECT_NO_THROW(top->Validate());
  top->AddArgument(C);
  EXPECT_THROW(top->Validate(), ValidationError);

  // VOTE/ATLEAST formula tests.
  top = FormulaPtr(new Formula("atleast"));
  top->vote_number(2);
  EXPECT_THROW(top->Validate(), ValidationError);
  top->AddArgument(A);
  EXPECT_THROW(top->Validate(), ValidationError);
  top->AddArgument(B);
  EXPECT_THROW(top->Validate(), ValidationError);
  top->AddArgument(C);
  EXPECT_NO_THROW(top->Validate());
}

TEST(GateTest, Inhibit) {
  BasicEventPtr A(new BasicEvent("a"));
  BasicEventPtr B(new BasicEvent("b"));
  BasicEventPtr C(new BasicEvent("c"));
  // INHIBIT Gate tests.
  Attribute inh_attr;
  inh_attr.name="flavor";
  inh_attr.value="inhibit";
  GatePtr top(new Gate("top"));
  FormulaPtr formula(new Formula("and"));
  top->formula(formula);
  top->AddAttribute(inh_attr);
  EXPECT_THROW(top->Validate(), ValidationError);
  top->formula()->AddArgument(A);
  EXPECT_THROW(top->Validate(), ValidationError);
  top->formula()->AddArgument(B);
  EXPECT_THROW(top->Validate(), ValidationError);

  top->formula()->AddArgument(C);
  EXPECT_THROW(top->Validate(), ValidationError);

  top = GatePtr(new Gate("top"));
  formula = FormulaPtr(new Formula("and"));  // Re-initialize.
  top->formula(formula);
  top->AddAttribute(inh_attr);

  Attribute cond;
  cond.name = "flavor";
  cond.value = "conditional";
  C->AddAttribute(cond);
  top->formula()->AddArgument(A);  // Basic event.
  top->formula()->AddArgument(C);  // Conditional event.
  EXPECT_NO_THROW(top->Validate());
  A->AddAttribute(cond);
  EXPECT_THROW(top->Validate(), ValidationError);
}

TEST(PrimaryEventTest, HouseProbability) {
  // House primary event.
  HouseEventPtr primary(new HouseEvent("valve"));
  EXPECT_FALSE(primary->state());  // Default state.
  // Setting with a valid values.
  EXPECT_NO_THROW(primary->state(true));
  EXPECT_TRUE(primary->state());
  EXPECT_NO_THROW(primary->state(false));
  EXPECT_FALSE(primary->state());
}
