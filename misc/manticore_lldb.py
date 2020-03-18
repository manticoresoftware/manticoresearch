# to use me add the command to ~/.lldbinit
# command script import "/path/to/manticore/misc/manticore_lldb.py"

# to debug run run directly:
# command script import "/path/to/manticore/misc/manticore_lldb.py" --allow-reload

class VecTraitsSynthProvider:

    def __init__(self, valobj, dict):
        self.valobj = valobj
        self.count = None

    def num_children(self):
        if self.count is None:
            self.count = self.num_children_impl()
        return self.count

    def num_children_impl(self):
        try:
            start_val = self.pData.GetValueAsUnsigned(0)

            # Make sure nothing is NULL
            if start_val == 0:
                return 0

            return self.iCount
        except:
            return 0

    def get_child_at_index(self, index):
        if index < 0:
            return None
        if index >= self.num_children():
            return None
        try:
            offset = index * self.data_size
            return self.pData.CreateChildAtOffset('[' + str(index) + ']', offset, self.data_type)
        except:
            return None

    def update(self):
        # preemptively setting this to None - we might end up changing our
        # mind later
        self.count = None
        try:
            self.pData = self.valobj.GetChildMemberWithName('m_pData')
            self.iCount = self.valobj.GetChildMemberWithName('m_iCount')
            self.data_type = self.pData.GetType().GetPointeeType()
            self.data_size = self.data_type.GetByteSize()
            # if any of these objects is invalid, it means there is no
            # point in trying to fetch anything
            if self.pData.IsValid() and self.data_type.IsValid():
                self.count = self.iCount.GetValueAsUnsigned()
            else:
                self.count = 0
        except:
            pass
        return True

    def get_child_index(self, name):
        try:
            return int(name.lstrip('[').rstrip(']'))
        except:
            return -1

    def has_children(self):
        return True


def VecTraitsSummaryProvider(valobj, internal_dict):
    if valobj.IsValid():
        content = valobj.GetChildMemberWithName('(content)')
        if content.IsValid():
            return content.GetSummary()
        else:
            count = valobj.GetNonSyntheticValue().GetChildMemberWithName('m_iCount').GetValueAsUnsigned()
            return '{} elems'.format(str(count))

def VecSummaryProvider(valobj, internal_dict):
    if valobj.IsValid():
        content = valobj.GetChildMemberWithName('(content)')
        if content.IsValid():
            return content.GetSummary()
        else:
            count = valobj.GetNonSyntheticValue().GetChildMemberWithName('m_iCount').GetValueAsUnsigned()
            limit = valobj.GetNonSyntheticValue().GetChildMemberWithName('m_iLimit').GetValueAsUnsigned()
            return '{} ({}) elems'.format(str(count), str(limit))

def __lldb_init_module(debugger, unused):

    #Vec-like - VecTraits_T, CSphVector<*, CSphFixedVector<*, CSphTightVector<*
    debugger.HandleCommand('type synthetic add -x "VecTraits_T|CSph(Vector|FixedVector|TightVector)<" -l manticore_lldb.VecTraitsSynthProvider')

    # summary for Vectraits and FixedVector - num of elems; for usual vector - also limit
    debugger.HandleCommand('type summary add -x "VecTraits_T<|CSphFixedVector<" --summary-string "[${var%#}]"')
    debugger.HandleCommand('type summary add -x "(sph::Vector_T|CSph(Vector|TightVector))<" --summary-string "[${var%#}] (${var.m_iLimit})"')

    #debugger.HandleCommand('type summary add -x "CSphVector<" -F manticore_lldb.VecSummaryProvider')

    #CSphString
    debugger.HandleCommand('type summary add CSphString --summary-string "${var.m_sValue}"')
    debugger.HandleCommand('type summary add StringBuilder_c --summary-string "${var.m_szBuffer} ${var.m_iUsed} (${var.m_iSize})"')
    debugger.HandleCommand(
        'type summary add StringBuilder_c::LazyComma_c --summary-string "${var.m_sPrefix} ${var.m_sComma} ${var.m_sSuffix}"')

    #std::pair
    debugger.HandleCommand('type summary add -x "^std::(__1::)?pair<" --inline-children')
#    debugger.HandleCommand('type summary add -x "^std::(__1::)?pair<" --summary-string "${var.first}:${var.second}"')
    debugger.HandleCommand('type summary add --inline-children Comma_c')
    debugger.HandleCommand('type summary add Str_t --summary-string "${var.first}"')

    debugger.HandleCommand('type summary add --inline-children CSphMatch')

