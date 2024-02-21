#!/usr/bin/bash
# author: silent
# date: 24/02/18
# version: 2

# Function to determine CPU fan duty based on temperature
get_cpu_fan_duty() {
  local temp=$1
  # CPU-specific fan duty logic
  if [ "$temp" -ge 90 ]; then
    echo 95
  elif [ "$temp" -ge 85 ]; then
    echo 80
  elif [ "$temp" -ge 80 ]; then
    echo 60
  elif [ "$temp" -ge 75 ]; then
    echo 45
  elif [ "$temp" -ge 70 ]; then
    echo 35
  elif [ "$temp" -ge 65 ]; then
    echo 25
  elif [ "$temp" -ge 60 ]; then
    echo 18
  elif [ "$temp" -ge 55 ]; then
    echo 18
  else
    echo 18
  fi
}

# Function to determine GPU fan duty based on temperature
get_gpu_fan_duty() {
  local temp=$1
  if [ "$temp" -ge 90 ]; then
    echo 95
  elif [ "$temp" -ge 85 ]; then
    echo 80
  elif [ "$temp" -ge 80 ]; then
    echo 25
  elif [ "$temp" -ge 75 ]; then
    echo 25
  elif [ "$temp" -ge 70 ]; then
    echo 18
  elif [ "$temp" -ge 65 ]; then
    echo 18
  elif [ "$temp" -ge 60 ]; then
    echo 18
  elif [ "$temp" -ge 55 ]; then
    echo 18
  else
    echo 18
  fi
}

if [ "$1" == "watch" ]; then
  while true; do
    GPU_TEMP=$(nvidia-smi -q -d=TEMPERATURE | grep 'GPU Current Temp' | awk '{print $5}')
    CPU_TEMP=$(/usr/local/bin/fanctrl | grep 'CPU Temp' | awk '{print $3}')
    
    CUR_CPU_FAN=$(fanctrl | grep 'FAN1 Duty' | awk '{print $3}')
    CUR_GPU_FAN=$(fanctrl | grep 'FAN2 Duty' | awk '{print $3}')
    CUR_CPU_FAN_RPM=$(fanctrl | grep 'FAN1 RPMs' | awk '{print $3}')
    CUR_GPU_FAN_RPM=$(fanctrl | grep 'FAN2 RPMs' | awk '{print $3}')
    
    echo "-----$(date)-----"
    echo "CPU_TEMP: $CPU_TEMP, CPU FAN DUTY: $CUR_CPU_FAN%, CPU FAN RPMs: $CUR_CPU_FAN_RPM"
    echo "GPU_TEMP: $GPU_TEMP, GPU FAN DUTY: $CUR_GPU_FAN%, GPU FAN RPMs: $CUR_GPU_FAN_RPM"
    
    sleep 1
  done
else
  while true; do
    # Get current temperatures
    GPU_TEMP=$(nvidia-smi -q -d=TEMPERATURE | grep 'GPU Current Temp' | awk '{print $5}')
    CPU_TEMP=$(/usr/local/bin/fanctrl | grep 'CPU Temp' | awk '{print $3}')
    
    # Determine fan duties
    CPU_FAN_DUTY=$(get_cpu_fan_duty $CPU_TEMP)
    GPU_FAN_DUTY=$(get_gpu_fan_duty $GPU_TEMP)
    
    # Set fan speeds
    /usr/local/bin/fanctrl 1 $CPU_FAN_DUTY > /dev/null
    /usr/local/bin/fanctrl 2 $GPU_FAN_DUTY > /dev/null
  
    CUR_CPU_FAN=$(fanctrl | grep 'FAN1 Duty' | awk '{print $3}')
    CUR_GPU_FAN=$(fanctrl | grep 'FAN2 Duty' | awk '{print $3}')
    CUR_CPU_FAN_RPM=$(fanctrl | grep 'FAN1 RPMs' | awk '{print $3}')
    CUR_GPU_FAN_RPM=$(fanctrl | grep 'FAN2 RPMs' | awk '{print $3}')
    
    echo "-----$(date)-----"
    echo "CPU_TEMP: $CPU_TEMP, CPU FAN DUTY: $CUR_CPU_FAN%, CPU FAN RPMs: $CUR_CPU_FAN_RPM"
    echo "GPU_TEMP: $GPU_TEMP, GPU FAN DUTY: $CUR_GPU_FAN%, GPU FAN RPMs: $CUR_GPU_FAN_RPM"
    
    sleep 1
  done
fi
