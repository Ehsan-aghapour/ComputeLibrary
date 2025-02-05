#!/usr/bin/env bash

#example usage:
#./mem.sh graph_googlenet_pipeline

#PROCESS_NAME="graph_googlenet_pipeline"
PROCESS_NAME=$1
PEAK_RSS=0

#./run_command.sh &

while :; do
  PID="$(adb shell pidof "$PROCESS_NAME" | tr -d '\r')"
  if [ -n "$PID" ]; then
    echo "Found PID $PID"
    break
  fi
  sleep 1
done

echo "Starting memory monitoring..."
while :; do
  still_running="$(adb shell pidof "$PROCESS_NAME" | tr -d '\r')"
  if [ -z "$still_running" ]; then
    echo "Process has ended. Stopping memory monitoring."
    break
  fi

  mem_info="$(adb shell cat /proc/$PID/status 2>/dev/null || true)"
  if [ -z "$mem_info" ]; then
    echo "Unable to read /proc/$PID/status. Stopping."
    break
  fi

  # Parse the lines we care about
  #current_rss_kb=$(echo "$mem_info" | grep VmRSS | awk '{print $2}')
  current_rss_kb=$(echo "$mem_info" | grep VmPeak | awk '{print $2}')
  hwm_rss_kb=$(echo "$mem_info" | grep VmHWM | awk '{print $2}')
  
  # Update local peak if we want to track it ourselves
  if [[ -n "$current_rss_kb" && "$current_rss_kb" -gt "$PEAK_RSS" ]]; then
    PEAK_RSS="$current_rss_kb"
  fi

  echo "Current RSS: $current_rss_kb kB, HWM (kernel peak): $hwm_rss_kb kB"
  
  sleep 1
done

echo "Local peak RSS was $PEAK_RSS kB"
echo "Done."
