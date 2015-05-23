#include <gtest/gtest.h>

#include "risk_analysis_tests.h"

// Benchmark Tests for auto-generated 200 event fault tree.
// Test Minimal cut sets and probability.
TEST_F(RiskAnalysisTest, 200Event) {
  std::string tree_input = "./share/scram/input/benchmark/200_event.xml";
  ran->AddSettings(settings.probability_analysis(true).limit_order(15)
                           .num_sums(3));
  ASSERT_NO_THROW(ran->ProcessInput(tree_input));
  ASSERT_NO_THROW(ran->Analyze());
  EXPECT_NEAR(0.5688586, p_total(), 1e-5);
  // Minimal cut set check.
  EXPECT_EQ(287, min_cut_sets().size());
}
