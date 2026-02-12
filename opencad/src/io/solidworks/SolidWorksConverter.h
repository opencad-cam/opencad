#pragma once

#include <string>

namespace opencad {
namespace io {

class SolidWorksConverter {
public:
  SolidWorksConverter();
  ~SolidWorksConverter();

  // Converts a SolidWorks file to STEP format using installed SW instance.
  // Returns true if successful, false otherwise.
  // resultingStepFile is populated with the path to the temp file.
  bool convertToStep(const std::string &inputFile,
                     std::string &resultingStepFile);

  std::string errorMessage() const;

private:
  std::string m_error;
};

} // namespace io
} // namespace opencad
