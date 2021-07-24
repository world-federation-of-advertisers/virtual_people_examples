// Copyright 2021 The Cross-Media Measurement Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// This is a tool to apply Virtual People Labeler to a set of input events,
// and write the output virtual people and aggregated reach report.
// Example usage:
// To applie the model represented by root node
//   bazel build -c opt //src/main/cc/wfa/virtual_people/model_applier
//   bazel-bin/src/main/cc/wfa/virtual_people/model_applier/model_applier \
//   --model_node_path=/tmp/model_applier/model_node.txt \
//   --input_path=/tmp/model_applier/input_events.txt \
//   --output_dir=/tmp/model_applier
// To applie the model represented by a list of nodes
//   bazel build -c opt //src/main/cc/wfa/virtual_people/model_applier
//   bazel-bin/src/main/cc/wfa/virtual_people/model_applier/model_applier \
//   --model_nodes_path=/tmp/model_applier/model_nodes \
//   --input_path=/tmp/model_applier/input_events \
//   --output_dir=/tmp/model_applier

#include <fcntl.h>
#include <filesystem>

#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "glog/logging.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "google/protobuf/message.h"
#include "google/protobuf/text_format.h"
#include "src/main/cc/wfa/virtual_people/model_applier/model_applier.pb.h"
#include "src/main/proto/wfa/virtual_people/common/model.pb.h"
#include "wfa/virtual_people/core/labeler/labeler.h"

ABSL_FLAG(std::string, model_node_path, "",
          "Path to the virtual people model file, contains textproto of "
          "CompiledNode. This represents the root node of the model tree, and "
          "all nodes in the model tree are referenced by CompiledNode "
          "directly. At least one of model_node_path and model_nodes_path must "
          "be set.");
ABSL_FLAG(std::string, model_nodes_path, "",
          "Path to the virtual people model file, contains textproto of "
          "CompiledNodeList. Nodes in the model tree are allowed to be "
          "referenced by indexes. At least one of model_node_path and "
          "model_nodes_path must be set.");
ABSL_FLAG(std::string, input_path, "",
          "Path to the input events, contains textproto of LabelerInputList.");
ABSL_FLAG(std::string, output_dir, "",
          "Path to the output directory.");

constexpr char kOutputEventsFilename[] = "output_events.txt";
constexpr char kOutputReportFilename[] = "output_reports.txt";

namespace wfa_virtual_people {

// Read textproto from file 
void ReadTextProtoFile(
    absl::string_view path, google::protobuf::Message& message) {
  int fd = open(path.data(), O_RDONLY);
  CHECK(fd > 0) << "Unable to open file: " << path;
  google::protobuf::io::FileInputStream file_input(fd);
  file_input.SetCloseOnDelete(true);
  CHECK(google::protobuf::TextFormat::Parse(&file_input, &message))
      << "Unable to parse textproto file: " << path;
}

// Write textproto to file 
void WriteTextProtoFile(
    absl::string_view path, const google::protobuf::Message& message) {
  // The output file is only accessible by owner.
  int fd = open(path.data(), O_CREAT | O_WRONLY, S_IRWXU);
  CHECK(fd > 0) << "Unable to create file: " << path;
  google::protobuf::io::FileOutputStream file_output(fd);
  file_output.SetCloseOnDelete(true);
  CHECK(google::protobuf::TextFormat::Print(message, &file_output))
      << "Unable to write textproto file: " << path;
}

// Create Labeler from the given model.
// If model_node_path is set, the model is represented as the single root node,
// in CompiledNode textproto.
// If model_nodes_path is set, the model is represented as a list of nodes, in
// CompiledNodeList textproto.
std::unique_ptr<Labeler> GetLabeler(
    absl::string_view model_node_path, absl::string_view model_nodes_path) {
  if (!model_node_path.empty()) {
    CompiledNode root;
    ReadTextProtoFile(model_node_path, root);
    absl::StatusOr<std::unique_ptr<Labeler>> labeler = Labeler::Build(root);
    CHECK(labeler.ok())
        << "Creating Labler failed with status: " << labeler.status();
    return *std::move(labeler);
  }
  if (!model_nodes_path.empty()) {
    CompiledNodeList node_list;
    ReadTextProtoFile(model_nodes_path, node_list);
    std::vector<CompiledNode> nodes(
        node_list.nodes().begin(), node_list.nodes().end());
    absl::StatusOr<std::unique_ptr<Labeler>> labeler = Labeler::Build(nodes);
    CHECK(labeler.ok())
        << "Creating Labler failed with status: " << labeler.status();
    return *std::move(labeler);
  }
  LOG(FATAL) << "Neither model_node_path nor model_nodes_path is set.";
}

// Read a list of input events, in LabelerInputList textproto.
LabelerInputList GetInputEvents(absl::string_view input_path) {
  CHECK(!input_path.empty()) << "input_path is not set.";
  LabelerInputList labeler_inputs;
  ReadTextProtoFile(input_path, labeler_inputs);
  return labeler_inputs;
}

LabelerOutputList ApplyLabeler(
    const Labeler& labeler, const LabelerInputList& labeler_inputs) {
  LabelerOutputList labeler_outputs;
  for (const LabelerInput& input : labeler_inputs.inputs()) {
    absl::Status status = labeler.Label(input, *labeler_outputs.add_outputs());
    CHECK(status.ok()) << "Labeling failed with status: " << status;
  }
  return labeler_outputs;
}

// Represent a row in aggregated report.
// @count represents the number of added virtual person.
// @virtual_person_ids represents the set of unique virtual person ids.
struct AggregatedRow {
  int64_t count = 0;
  absl::flat_hash_set<int64_t> virtual_person_ids;

  void AddVirtualPeople(const int64_t virtual_person_id) {
    ++count;
    virtual_person_ids.insert(virtual_person_id);
  }
};

// Aggregate the output virtual people to total impressions/reach, and
// impressions/reach by label.
AggregatedReport AggregateOutput(const LabelerOutputList& labeler_outputs) {
  // Map from PersonLabelAttributes to count and virtual person ids set.
  // Key is the serialized string of PersonLabelAttributes.
  absl::flat_hash_map<std::string, AggregatedRow> label_rows;
  // The aggregated counts and virtual person ids set for all virtual people.
  AggregatedRow total;
  for (const LabelerOutput& output : labeler_outputs.outputs()) {
    for (const VirtualPersonActivity& person : output.people()) {
      if (person.has_label()) {
        std::string label = person.label().SerializeAsString();
        label_rows[label].AddVirtualPeople(person.virtual_person_id());
      }
      total.AddVirtualPeople(person.virtual_person_id());
    }
  }

  AggregatedReport report;
  AggregatedReport::Row* total_row = report.add_rows();
  total_row->set_impressions(total.count);
  total_row->set_reach(total.virtual_person_ids.size());
  for (const auto& label_row : label_rows) {
    AggregatedReport::Row* row = report.add_rows();
    CHECK(row->mutable_attrs()->ParseFromString(label_row.first))
        << "Unable to parse string to PersonLabelAttributes: "
        << label_row.first;
    row->set_impressions(label_row.second.count);
    row->set_reach(label_row.second.virtual_person_ids.size());
  }

  return report;
}

// Write the labeler output and aggregated report to @output_dir.
// Create the directory if not exists.
void WriteOutput(
    absl::string_view output_dir,
    const LabelerOutputList& labeler_outputs,
    const AggregatedReport& report) {
  CHECK(!output_dir.empty()) << "output_dir is not set.";

  if (!std::filesystem::exists(output_dir)) {
    CHECK(std::filesystem::create_directory(output_dir))
        << "Failed to create directory: " << output_dir;
  }

  WriteTextProtoFile(
      absl::StrCat(output_dir, "/", kOutputEventsFilename), labeler_outputs);
  WriteTextProtoFile(
      absl::StrCat(output_dir, "/", kOutputReportFilename), report);
}

}  // namespace wfa_virtual_people

int main(int argc, char** argv) {
  absl::ParseCommandLine(argc, argv);
  google::InitGoogleLogging(argv[0]);

  std::unique_ptr<wfa_virtual_people::Labeler> labeler =
      wfa_virtual_people::GetLabeler(
          absl::GetFlag(FLAGS_model_node_path),
          absl::GetFlag(FLAGS_model_nodes_path));
  
  wfa_virtual_people::LabelerInputList labeler_inputs =
      wfa_virtual_people::GetInputEvents(absl::GetFlag(FLAGS_input_path));

  wfa_virtual_people::LabelerOutputList labeler_outputs =
      wfa_virtual_people::ApplyLabeler(*labeler, labeler_inputs);

  wfa_virtual_people::AggregatedReport report =
      wfa_virtual_people::AggregateOutput(labeler_outputs);
  
  wfa_virtual_people::WriteOutput(
      absl::GetFlag(FLAGS_output_dir), labeler_outputs, report);

  return 0;
}
