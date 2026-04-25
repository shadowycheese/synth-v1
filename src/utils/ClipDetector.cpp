#include "ClipDetector.h"

ClipDetector::ClipDetector(char const *name, AudioStream &source) : patch(source, 0, analyzer, 0)
{
    strcpy(_name, name);

    ClipDetectorTask::Add(this);
}

ClipDetector *ClipDetectorTask::_detectors[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};