#include "StreamSections.hpp"


void StreamSection::SkipBytes(std::stringstream& ss, unsigned long len)
{
    unsigned long offset = (unsigned long) ss.tellg() + len;
    ss.seekg(offset);
}

void StreamSection::RevertBytes(std::stringstream& ss,
                                unsigned long capacity)
{
    unsigned long offset = (unsigned long) ss.tellg() - capacity;
    ss.seekg(offset);
}

void PacketHeader::Print() const
{
    std::cout   << std::hex <<
        "SB="   << sync_byte.to_ulong() <<
        " E="   << transport_error_indicator.to_ulong() <<
        " PUS=" << payload_unit_start_indicator.to_ulong() <<
        " Pr="  << transport_priority.to_ulong() <<
        " PID=" << PID.to_ulong() <<
        " TSC=" << transport_scrambling_control.to_ulong() <<
        " AF="  << adaptation_field_control.to_ulong() <<
        " CC="  << continuity_counter.to_ulong() <<
    std::endl;
}

bool PacketHeader::HasAdaptationField()
{
    if (adaptation_field_control == 0b10 ||
        adaptation_field_control == 0b11)
        return true;
    return false;
}

bool PacketHeader::HasPSI()
{
    if (PID == 0) {
        is_PAT = true;
        PID_in_range = false;
        return true;
    }
    else if (PID.to_ulong() >= 0x0010 && PID.to_ulong() <= 0x1FFE) {
        is_PAT = false;
        PID_in_range = true;
        return true;
    }
    return false;
}

bool PacketHeader::Set(std::stringstream& ss)
{
    ss >> sync_byte;
    ss >> transport_error_indicator;
    ss >> payload_unit_start_indicator;
    ss >> transport_priority;
    ss >> PID;
    ss >> transport_scrambling_control;
    ss >> adaptation_field_control;
    ss >> continuity_counter;
    if (adaptation_field_control == ADAPT::ADAPT_ONLY ||
        adaptation_field_control == ADAPT::RESERVED)
        return false;
    
    if (adaptation_field_control == ADAPT::ADAPT_PAYLOAD) {
        ss >> adaptation_field_length;
        SkipBytes(ss, adaptation_field_length.to_ulong());
    }
    return HasPSI();
}

bool PSI::Set(std::stringstream& ss)
{
    ss >> pointer_field;
    if (pointer_field != 0)
        SkipBytes(ss, pointer_field.to_ulong());
    return true;
}

void PSI::SetStartTable(std::stringstream& ss)
{
    ss >> table_id;
    ss >> section_syntax_indicator;
    ss >> _zero;
    ss >> reserved;
    ss >> section_length;
}

int PAT::GetProgramSectionSize() const
{
    return section_length.to_ulong()
        - (transport_stream_id.size()
           + reserved0.size()
           + version_number.size()
           + current_next_indicator.size()
           + section_number.size()
           + last_section_number.size()
           + CRC_32.size()
           ) / 8;
}

void PAT::AddPrograms(std::stringstream& ss,
                      std::set<unsigned long>& programs_PIDs)
{
    SetStartTable(ss);
    if (table_id != TABLE_ID::TABLE_PAT)
        return;
    ss >> transport_stream_id;
    ss >> reserved0;
    ss >> version_number;
    ss >> current_next_indicator;
    ss >> section_number;
    ss >> last_section_number; 

    int section_size = GetProgramSectionSize();
    for (; section_size > 0; section_size -= 4) {
        ss >> program_number;
        ss >> reserved1;
        if (program_number == 0) {
            ss >> network_PID;
        }
        else {
            ss >> program_map_PID;
            programs_PIDs.insert(program_number.to_ulong());
        }
    }
    ss >> CRC_32;
}

void PMT::PrintES_Info(std::stringstream& ss,
                       std::set<unsigned long>& programs_PIDs,
                       std::set<unsigned long>& es_set)
{
    SetStartTable(ss);
    if (table_id != TABLE_ID::TABLE_PMT)
        return;
    ss >> program_number;
    ss >> reserved0;
    ss >> version_number;
    ss >> current_next_indicator;
    ss >> section_number;
    ss >> last_section_number;
    ss >> reserved1;
    ss >> PCR_PID;
    ss >> reserved2;
    ss >> program_info_length;

    ss >> stream_type;
    ss >> reserved3;
    ss >> elementary_PID;
    ss >> reserved4;
    ss >> ES_info_length;

    auto program_search = programs_PIDs.find(program_number.to_ulong());
    if (program_search != programs_PIDs.end()) {
        auto es_search = es_set.find(elementary_PID.to_ulong());
        if (es_search == es_set.end()) {
            es_set.insert(elementary_PID.to_ulong());
            Print();
        }
    }
}

void PMT::Print() const
{
    std::cout << "Elementary PID: "
              << elementary_PID.to_ulong() << '\n';
    std::cout << "ES info length: "
              << ES_info_length.to_ulong() << '\n';
    std::cout << "Stream Type: " << std::hex
              << stream_type.to_ulong() << '\n';
    std::cout << '\n' << std::dec;
}

int PMT::GetProgramSectionSize() const
{
    return section_length.to_ulong()
        - (program_number.size()
           + reserved0.size()
           + version_number.size()
           + current_next_indicator.size()
           + section_number.size()
           + last_section_number.size()
           + reserved1.size()
           + PCR_PID.size()
           + reserved2.size()
           + program_info_length.size()
           + CRC_32.size()
          ) / 8;
}

int PMT::GetNestedSectionSize() const
{
    return ES_info_length.to_ulong()
        + (stream_type.size()
           + reserved3.size()
           + elementary_PID.size()
           + reserved4.size()
           + ES_info_length.size()
          ) / 8;
}