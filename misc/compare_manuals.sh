#!/bin/bash

# Script to compare manual files across different languages
# Usage: ./compare_manuals.sh

set -euo pipefail

# Global variables for tracking issues and options
HAS_ISSUES=0
REMOVE_MODE=false

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Base directory for manuals
MANUAL_DIR="manual"
ENGLISH_DIR="${MANUAL_DIR}/english"

# Function to print colored output
print_colored() {
    local color=$1
    local message=$2
    echo -e "${color}${message}${NC}"
}

# Function to get all language directories
get_language_dirs() {
    find "${MANUAL_DIR}" -mindepth 1 -maxdepth 1 -type d | grep -v "${ENGLISH_DIR}" | sort
}

# Function to get all .md files and directories relative to base path
get_structure() {
    local base_dir=$1
    if [[ ! -d "$base_dir" ]]; then
        return 1
    fi

    # Get all .md files and directories
    find "$base_dir" -name "*.md" -o -type d | \
    sed "s|^${base_dir}/||" | \
    grep -v "^$" | \
    sort
}

# Function to compare file content line by line
compare_files() {
    local file1=$1
    local file2=$2

    if [[ ! -f "$file1" ]] || [[ ! -f "$file2" ]]; then
        return 1
    fi

    local f1_lines f2_lines
    f1_lines=$(wc -l < "$file1")
    f2_lines=$(wc -l < "$file2")

    # First check line count
    if [[ $f1_lines -ne $f2_lines ]]; then
        echo "$f1_lines:$f2_lines"
        return 1
    fi

    # Check empty/non-empty line alignment
    local line_num=1
    local misaligned_lines=()
    
    while IFS= read -r line1 && IFS= read -r line2 <&3; do
        # Remove \r and check if line is empty
        local clean_line1="${line1//$'\r'/}"
        local clean_line2="${line2//$'\r'/}"
        
        local is_empty1=false
        local is_empty2=false
        
        [[ -z "$clean_line1" ]] && is_empty1=true
        [[ -z "$clean_line2" ]] && is_empty2=true
        
        # Check if empty/non-empty state matches
        if [[ "$is_empty1" != "$is_empty2" ]]; then
            misaligned_lines+=("$line_num")
        fi
        
        ((line_num++))
    done < "$file1" 3< "$file2"
    
    # Return misaligned lines if any found
    if [[ ${#misaligned_lines[@]} -gt 0 ]]; then
        echo "alignment:$(IFS=,; echo "${misaligned_lines[*]}")"
        return 1
    fi

    # Check HTML comments are identical (not translated)
    local comments1 comments2
    comments1=$(grep -o '<!--.*-->' "$file1" | sort)
    comments2=$(grep -o '<!--.*-->' "$file2" | sort)
    
    if [[ "$comments1" != "$comments2" ]]; then
        echo "comments:translated"
        return 1
    fi

    return 0
}

# Function to check if path exists in language directory
path_exists() {
    local lang_dir=$1
    local relative_path=$2

    local full_path="${lang_dir}/${relative_path}"
    [[ -e "$full_path" ]]
}

# Main comparison function
compare_with_languages() {
    local english_dir=$1

    if [[ ! -d "$english_dir" ]]; then
        print_colored "$RED" "Error: English directory '$english_dir' not found!"
        exit 1
    fi

    print_colored "$BLUE" "=== Manual Structure Comparison ==="
    print_colored "$BLUE" "Base directory: $english_dir"
    if [[ "$REMOVE_MODE" == "true" ]]; then
        print_colored "$YELLOW" "REMOVE MODE: Files with mismatched line counts will be automatically removed"
    fi
    echo

    # Get English structure
    local english_structure
    english_structure=$(get_structure "$english_dir")

    if [[ -z "$english_structure" ]]; then
        print_colored "$YELLOW" "Warning: No .md files or directories found in $english_dir"
        return
    fi

    # Get all language directories
    local lang_dirs
    lang_dirs=$(get_language_dirs)

    if [[ -z "$lang_dirs" ]]; then
        print_colored "$YELLOW" "Warning: No other language directories found in $MANUAL_DIR"
        return
    fi

    print_colored "$GREEN" "Found language directories:"
    echo "$lang_dirs" | while read -r lang_dir; do
        echo "  - $(basename "$lang_dir")"
    done
    echo

    # Compare each language with English
    while IFS= read -r lang_dir; do
        local lang_name
        lang_name=$(basename "$lang_dir")

        print_colored "$BLUE" "--- Comparing with $lang_name ---"

        local missing_files=()
        local different_files=()
        local missing_dirs=()
        local removed_files=()

        # Check each item in English structure
        while IFS= read -r item; do
            local english_path="${english_dir}/${item}"
            local lang_path="${lang_dir}/${item}"

            if [[ ! -e "$lang_path" ]]; then
                if [[ -d "$english_path" ]]; then
                    missing_dirs+=("$item")
                    HAS_ISSUES=1
                elif [[ -f "$english_path" ]]; then
                    missing_files+=("$item")
                    HAS_ISSUES=1
                fi
            elif [[ -f "$english_path" ]] && [[ -f "$lang_path" ]]; then
                # Both are files, compare content
                local diff_result
                set +e  # Temporarily disable exit on error
                diff_result=$(compare_files "$english_path" "$lang_path")
                local compare_exit_code=$?
                set -e  # Re-enable exit on error

                if [[ $compare_exit_code -ne 0 ]]; then
                    HAS_ISSUES=1
                    if [[ "$REMOVE_MODE" == "true" ]]; then
                        # Remove the file with mismatched line count
                        rm -f "$lang_path"
                        removed_files+=("$item")
                        print_colored "$RED" "REMOVED: $lang_path (line count mismatch)"
                    else
                        if [[ -n "$diff_result" ]]; then
                            different_files+=("$item|$diff_result")
                        else
                            different_files+=("$item")
                        fi
                    fi
                fi
            fi
        done <<< "$english_structure"

        # Report results
        if [[ ${#missing_dirs[@]} -gt 0 ]]; then
            print_colored "$RED" "Missing directories in $lang_name:"
            printf '  %s\n' "${missing_dirs[@]}"
            echo
        fi

        if [[ ${#missing_files[@]} -gt 0 ]]; then
            print_colored "$RED" "Missing files in $lang_name:"
            printf '  %s\n' "${missing_files[@]}"
            echo
        fi

        if [[ ${#removed_files[@]} -gt 0 ]]; then
            print_colored "$RED" "Removed files with mismatched line counts in $lang_name:"
            printf '  %s\n' "${removed_files[@]}"
            echo
        fi

        if [[ ${#different_files[@]} -gt 0 ]]; then
            print_colored "$YELLOW" "Files with different content in $lang_name:"
            for file_info in "${different_files[@]}"; do
                if [[ "$file_info" == *"|"* ]]; then
                    local file="${file_info%|*}"
                    local error_info="${file_info#*|}"
                    if [[ "$error_info" == *":"* ]] && [[ "$error_info" != "alignment:"* ]] && [[ "$error_info" != "comments:"* ]]; then
                        # Line count mismatch
                        local eng_lines="${error_info%:*}"
                        local lang_lines="${error_info#*:}"
                        printf '  %s (English: %d lines, %s: %d lines)\n' "$file" "$eng_lines" "$lang_name" "$lang_lines"
                    elif [[ "$error_info" == "alignment:"* ]]; then
                        # Empty/non-empty line alignment issue
                        local misaligned_lines="${error_info#alignment:}"
                        printf '  %s (empty/non-empty line alignment mismatch at lines: %s)\n' "$file" "$misaligned_lines"
                    elif [[ "$error_info" == "comments:"* ]]; then
                        # HTML comments translated
                        printf '  %s (HTML comments <!-- --> were translated, must remain in English)\n' "$file"
                    else
                        printf '  %s (%s)\n' "$file" "$error_info"
                    fi
                else
                    printf '  %s\n' "$file_info"
                fi
            done
            echo
        fi

        if [[ ${#missing_dirs[@]} -eq 0 ]] && [[ ${#missing_files[@]} -eq 0 ]] && [[ ${#different_files[@]} -eq 0 ]] && [[ ${#removed_files[@]} -eq 0 ]]; then
            print_colored "$GREEN" "✓ $lang_name structure and content matches English perfectly!"
            echo
        fi
    done <<< "$lang_dirs"
}

# Function to show detailed diff for specific files
show_detailed_diff() {
    local english_dir=$1
    local lang_dir=$2
    local file_path=$3

    local english_file="${english_dir}/${file_path}"
    local lang_file="${lang_dir}/${file_path}"

    if [[ -f "$english_file" ]] && [[ -f "$lang_file" ]]; then
        print_colored "$BLUE" "--- Detailed diff for $file_path ---"
        print_colored "$BLUE" "English: $english_file"
        print_colored "$BLUE" "$(basename "$lang_dir"): $lang_file"
        echo

        # Show side-by-side diff with line numbers
        diff -u "$english_file" "$lang_file" || true
        echo
    fi
}

# Function to show summary statistics
show_summary() {
    local english_dir=$1

    print_colored "$BLUE" "=== Summary Statistics ==="

    # Count English files and directories
    local english_files
    local english_dirs
    english_files=$(find "$english_dir" -name "*.md" | wc -l | tr -d ' ')
    english_dirs=$(find "$english_dir" -type d | wc -l | tr -d ' ')

    echo "English structure:"
    echo "  - Directories: $english_dirs"
    echo "  - .md files: $english_files"
    echo

    # Count for each language
    get_language_dirs | while read -r lang_dir; do
        local lang_name
        lang_name=$(basename "$lang_dir")

        local lang_files
        local lang_dirs_count
        lang_files=$(find "$lang_dir" -name "*.md" 2>/dev/null | wc -l | tr -d ' ')
        lang_dirs_count=$(find "$lang_dir" -type d 2>/dev/null | wc -l | tr -d ' ')

        echo "$lang_name structure:"
        echo "  - Directories: $lang_dirs_count"
        echo "  - .md files: $lang_files"
        echo
    done
}

# Main execution
main() {
    # Check if manual directory exists
    if [[ ! -d "$MANUAL_DIR" ]]; then
        print_colored "$RED" "Error: Manual directory '$MANUAL_DIR' not found in current directory!"
        print_colored "$YELLOW" "Current directory: $(pwd)"
        exit 1
    fi

    # Check if English directory exists
    if [[ ! -d "$ENGLISH_DIR" ]]; then
        print_colored "$RED" "Error: English directory '$ENGLISH_DIR' not found!"
        exit 1
    fi

    print_colored "$GREEN" "Starting manual comparison..."
    echo "Current directory: $(pwd)"
    echo

    # Show summary first
    show_summary "$ENGLISH_DIR"

    # Compare structures and content
    # Compare structures and content
    compare_with_languages "$ENGLISH_DIR"

    if [[ $HAS_ISSUES -eq 0 ]]; then
        print_colored "$GREEN" "Comparison completed! All translations are perfect."
        exit 0
    else
        print_colored "$RED" "Comparison completed with issues found."
        exit 1
    fi
}

# Handle command line arguments
# Handle command line arguments
case "${1:-}" in
    -h|--help)
        echo "Usage: $0 [OPTIONS]"
        echo ""
        echo "Compare manual files across different languages"
        echo ""
        echo "OPTIONS:"
        echo "  -h, --help     Show this help message"
        echo "  -s, --summary  Show only summary statistics"
        echo "  --remove       Automatically remove files with mismatched line counts"
        echo ""
        echo "EXIT CODES:"
        echo "  0              All translations perfect (no missing files or line count mismatches)"
        echo "  1              Issues found (missing translations or line count mismatches)"
        echo ""
        echo "This script compares the structure and content of .md files"
        echo "in manual/english/ with all other language directories."
        echo "With --remove flag, files with different line counts are automatically deleted."
        exit 0
        ;;
    -s|--summary)
        show_summary "$ENGLISH_DIR"
        exit 0
        ;;
    --remove)
        REMOVE_MODE=true
        main
        ;;
    "")
        main
        ;;
    *)
        print_colored "$RED" "Unknown option: $1"
        print_colored "$YELLOW" "Use -h or --help for usage information"
        exit 1
        ;;
esac
