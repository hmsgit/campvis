// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2015, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universitaet Muenchen
//      Boltzmannstr. 3, 85748 Garching b. Muenchen, Germany
// 
// For a full list of authors and contributors, please refer to the file "AUTHORS.txt".
// 
// Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file 
// except in compliance with the License. You may obtain a copy of the License at
// 
// http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software distributed under the 
// License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, 
// either express or implied. See the License for the specific language governing permissions 
// and limitations under the License.
// 
// ================================================================================================

#ifndef DATASERIES_H__
#define DATASERIES_H__

#include "core/datastructures/abstractdata.h"
#include "core/datastructures/datahandle.h"
#include <vector>

namespace campvis {

    /**
     * Class encapsulating a series of AbstractData objects.
     */
    class CAMPVIS_CORE_API DataSeries : public AbstractData {
    public:
        /**
         * Constructor
         */
        DataSeries();

        /**
         * Virtual destructor
         */
        virtual ~DataSeries();


        /**
         * Prototype - clone method, some people call this virtual constructor...
         * \return  A copy of this object.
         */
        virtual DataSeries* clone() const;

        /**
         * Returns the local memory footprint of the data in bytes.
         * \return  Number of bytes occupied in local memory by the data.
         */
        virtual size_t getLocalMemoryFootprint() const;

        /**
         * Returns the video memory footprint of the data in bytes.
         * \return  Number of bytes occupied in video memory by the data.
         */
        virtual size_t getVideoMemoryFootprint() const;

        /// \see AbstractData::getTypeAsString()
        virtual std::string getTypeAsString() const;
        
        /**
         * Appends the AbstractData instance \a data to the series.
         * \param   AbstractData   Data to be added.
         */
        void addData(AbstractData* data);
        
        /**
         * Appends the DataHandle \a dh to the series.
         * \note    The DataHandle must contain valid data.
         * \param   dh  DataHandle with data to be added.
         */
        void addData(DataHandle dh);

        /**
         * Returns the number of AbstractData instances in this series.
         * \return  _data.size()
         */
        size_t getNumDatas() const;

        /**
         * Returns a DataHandle with the AbstractData instance number \a index of this series.
         * \param   index   Index of the data to return
         * \return  _data[index]
         */
        DataHandle getData(size_t index) const;

    protected:
        std::vector<DataHandle> _data;    ///< the images of this series
    };

}

#endif // DATASERIES_H__