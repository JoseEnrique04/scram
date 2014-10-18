/// @file fault_tree_analysis.h
/// Fault Tree Analysis.
#ifndef SCRAM_SRC_FAULT_TREE_ANALYSIS_H_
#define SCRAM_SRC_FAULT_TREE_ANALYSIS_H_

#include <map>
#include <set>
#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>

#include "event.h"
#include "fault_tree.h"
#include "superset.h"

class FaultTreeAnalysisTest;
class PerformanceTest;

typedef boost::shared_ptr<scram::Event> EventPtr;
typedef boost::shared_ptr<scram::Gate> GatePtr;
typedef boost::shared_ptr<scram::PrimaryEvent> PrimaryEventPtr;

typedef boost::shared_ptr<scram::Superset> SupersetPtr;

typedef boost::shared_ptr<scram::FaultTree> FaultTreePtr;

namespace scram {

class Reporter;

/// @class FaultTreeAnalysis
/// Fault tree analysis functionality.
class FaultTreeAnalysis {
  friend class ::FaultTreeAnalysisTest;
  friend class ::PerformanceTest;
  friend class Reporter;

 public:
  /// The main constructor of Fault Tree Analysis.
  /// @param[in] limit_order The maximum limit on minimal cut sets' order.
  /// @throws ValueError if any of the parameters are invalid.
  explicit FaultTreeAnalysis(int limit_order = 20);

  /// Analyzes the fault tree and performs computations.
  /// This function must be called only after initilizing the tree with or
  /// without its probabilities. Underlying objects may throw errors
  /// if the fault tree has initialization issues. However, there is no
  /// quarantee for that.
  /// @param[in] fault_tree Valid Fault Tree.
  /// @note Cut set generator: O_avg(N) O_max(N)
  void Analyze(const FaultTreePtr& fault_tree);

  /// @returns Set with minimal cut sets.
  /// @note The user should make sure that the analysis is actually done.
  inline const std::set< std::set<std::string> >& min_cut_sets() {
    return min_cut_sets_;
  }

 private:
  /// Preprocesses the fault tree.
  /// Merges similar gates.
  /// @param[in] gate The starting gate to traverse the tree. This is for
  ///                 recursive purposes.
  void PreprocessTree(const GatePtr& gate);

  /// Traverses the fault tree and expands it into sets of gates and events.
  /// @param[in] set_with_gates A superset with gates.
  /// @param[in] cut_sets Container for cut sets upon tree expansion.
  void ExpandTree(const SupersetPtr& set_with_gates,
                  std::vector<SupersetPtr>* cut_sets);

  /// Expands the children of a top or intermediate event to Supersets.
  /// @param[in] inter_index The index number of the parent node.
  /// @param[out] sets The final Supersets from the children.
  /// @throws ValueError if the parent's gate is not recognized.
  /// @note The final sets are dependent on the gate of the parent.
  /// @note O_avg(N, N*logN) O_max(N^2, N^3*logN) where N is a children number.
  void ExpandSets(int inter_index, std::vector<SupersetPtr>* sets);

  /// Converts a set of children into a vector of children indices.
  /// @param[in] children A set of children of a gate.
  /// @param[out] events_children The same children's indices in a vector.
  void ConvertChildrenToVector(const std::map<std::string, EventPtr>& children,
                               std::vector<int>* events_children);

  /// Populates the sets of supersets of a gate that has already been expanded.
  /// @param[in] inter_index The index number of the parent node.
  /// @param[out] sets The final Supersets from the children if there is a gate.
  /// @returns true if sets already exist and got copied.
  /// @returns false if the gate is not yet encountered.
  /// @note This function works together with SaveExpandedSets.
  bool GetExpandedSets(int inter_index, std::vector<SupersetPtr>* sets);

  /// Saves the expanded sets in case the gate is repeated. The sets are
  /// saved in repeat_exp_ container.
  /// @param[in] inter_index The index number of the parent node.
  /// @param[in] sets The expanded Supersets from the children.
  /// @note This function works together with GetExpandedSets.
  void SaveExpandedSets(int inter_index, const std::vector<SupersetPtr>& sets);

  /// Expands positive gate's children into supersets.
  /// @param[in] inter_index The index number of the parent node.
  /// @param[in] events_children The indices of the children of the event.
  /// @param[out] sets The final Supersets from the children if there is a gate.
  void ExpandPositiveGate(int inter_index,
                          const std::vector<int>& events_children,
                          std::vector<SupersetPtr>* sets);

  /// Expands complement gate's children into supersets.
  /// @param[in] inter_index The index number of the parent node.
  /// @param[in] events_children The indices of the children of the event.
  /// @param[out] sets The final Supersets from the children if there is a gate.
  void ExpandNegativeGate(int inter_index,
                          const std::vector<int>& events_children,
                          std::vector<SupersetPtr>* sets);

  /// Expands sets for OR operator.
  /// @param[in] mult The positive(1) or negative(-1) event indicator.
  /// @param[in] events_children The indices of the children of the event.
  /// @param[out] sets The final Supersets generated for OR operator.
  /// @note O_avg(N) O_max(N^2)
  void SetOr(int mult, const std::vector<int>& events_children,
             std::vector<SupersetPtr>* sets);

  /// Expands sets for AND operator.
  /// @param[in] mult The positive(1) or negative(-1) event indicator.
  /// @param[in] events_children The indices of the children of the event.
  /// @param[out] sets The final Supersets generated for OR operator.
  /// @note O_avg(N*logN) O_max(N*logN) where N is the number of children.
  void SetAnd(int mult, const std::vector<int>& events_children,
              std::vector<SupersetPtr>* sets);

  /// Expands sets for XOR operator.
  /// @param[in] inter_index The positive or negative gate event indicator.
  /// @param[in] events_children The indices of the children of the event.
  /// @param[out] sets The final Supersets generated for OR operator.
  void SetXor(int inter_index, const std::vector<int>& events_children,
              std::vector<SupersetPtr>* sets);

  /// Expands sets for Vote or Atleat operator.
  /// @param[in] inter_index The positive or negative gate event indicator.
  /// @param[in] events_children The indices of the children of the event.
  /// @param[out] sets The final Supersets generated for OR operator.
  void SetAtleast(int inter_index, const std::vector<int>& events_children,
                  std::vector<SupersetPtr>* sets);

  /// Finds minimal cut sets from cut sets.
  /// Applys rule 4 to reduce unique cut sets to minimal cut sets.
  /// @param[in] cut_sets Cut sets with primary events.
  /// @param[in] mcs_lower_order Reference minimal cut sets of some order.
  /// @param[in] min_order The order of sets to become minimal.
  /// @param[out] imcs Min cut sets with indices of events.
  /// @note T_avg(N^3 + N^2*logN + N*logN) = O_avg(N^3)
  void FindMcs(const std::vector< const std::set<int>* >& cut_sets,
               const std::vector< std::set<int> >& mcs_lower_order,
               int min_order,
               std::vector< std::set<int> >* imcs);

  /// Assigns an index to each primary event, and then populates with this
  /// indices new databases of minimal cut sets and primary to integer
  /// converting maps.
  /// In addition, this function copies all events from
  /// the fault tree for future reference.
  /// @param[in] fault_tree Fault Tree with events and gates.
  /// @note O_avg(N) O_max(N^2) where N is the total number of tree nodes.
  void AssignIndices(const FaultTreePtr& fault_tree);

  /// Converts minimal cut sets from indices to strings for future reporting.
  /// @param[in] imcs Min cut sets with indices of events.
  void SetsToString(const std::vector< std::set<int> >& imcs);

  std::vector<PrimaryEventPtr> int_to_primary_;  ///< Indices to primary events.
  /// Indices of primary events.
  boost::unordered_map<std::string, int> primary_to_int_;

  int top_event_index_;  ///< The index of the top event.
  /// Intermediate events from indices.
  boost::unordered_map<int, GatePtr> int_to_inter_;
  /// Indices of intermediate events.
  boost::unordered_map<std::string, int> inter_to_int_;

  /// This member is used to provide any warnings about assumptions,
  /// calculations, and settings. These warnings must be written into output
  /// file.
  std::string warnings_;

  /// Limit on the size of the minimal cut sets for performance reasons.
  int limit_order_;

  /// Top event.
  GatePtr top_event_;

  /// Holder for intermediate events.
  boost::unordered_map<std::string, GatePtr> inter_events_;

  /// Container for primary events.
  boost::unordered_map<std::string, PrimaryEventPtr> primary_events_;

  /// Container for minimal cut sets.
  std::set< std::set<std::string> > min_cut_sets_;

  /// Maximum order of minimal cut sets.
  int max_order_;

  // Time logging
  double exp_time_;  ///< Expansion of tree gates time.
  double mcs_time_;  ///< Time for MCS generation.

  /// Track if the gates are repeated upon expansion.
  boost::unordered_map<int, std::vector<SupersetPtr> > repeat_exp_;
};

}  // namespace scram

#endif  // SCRAM_SRC_FAULT_TREE_ANALYSIS_H_
