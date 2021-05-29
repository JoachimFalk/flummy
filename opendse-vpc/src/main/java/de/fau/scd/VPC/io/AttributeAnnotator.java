// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:4; -*-
// vim: set sw=4 ts=8 sts=4 et:
/*
 * Copyright (c)
 *   2021 FAU -- Joachim Falk <joachim.falk@fau.de>
 *
 *   This library is free software; you can redistribute it and/or modify it under
 *   the terms of the GNU Lesser General Public License as published by the Free
 *   Software Foundation; either version 2 of the License, or (at your option) any
 *   later version.
 *
 *   This library is distributed in the hope that it will be useful, but WITHOUT
 *   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *   FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 *   details.
 *
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with this library; if not, write to the Free Software Foundation, Inc.,
 *   59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 */

package de.fau.scd.VPC.io;

import java.util.ArrayList;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import com.google.inject.Inject;

import de.fau.scd.VPC.config.properties.AttributeAnnotation;
import net.sf.opendse.model.Mapping;
import net.sf.opendse.model.Resource;
import net.sf.opendse.model.Specification;
import net.sf.opendse.model.Task;
import net.sf.opendse.optimization.io.SpecificationTransformer;

public class AttributeAnnotator implements SpecificationTransformer {

    private List<AttributeAnnotation> attrAnnotationList;

    interface AttributeAnnotations {
        public List<AttributeAnnotation> getAttributeAnnotation();
    }

    @Inject
    public AttributeAnnotator(AttributeAnnotations attributeAnnotations) {
        this.attrAnnotationList = attributeAnnotations.getAttributeAnnotation();
    }

    @Override
    public void transform(Specification specification) {

        List<AttributeAnnotation> applicationAttrAnnotations = new ArrayList<>();
        List<AttributeAnnotation> architectureAttrAnnotations = new ArrayList<>();
        List<AttributeAnnotation> mappingsAttrAnnotations = new ArrayList<>();
        List<AttributeAnnotation> specificationAttrAnnotations = new ArrayList<>();

        for (AttributeAnnotation aa : attrAnnotationList) {
            switch (aa.getNodeType()) {
                case APPLICATION:
                    applicationAttrAnnotations.add(aa);
                    break;
                case ARCHITECTURE:
                    architectureAttrAnnotations.add(aa);
                    break;
                case MAPPINGS:
                    mappingsAttrAnnotations.add(aa);
                    break;
                case SPECIFICATION:
                    if (aa.getElemRegex().pattern().isEmpty()) {
                        specificationAttrAnnotations.add(aa);
                    } else {
                        applicationAttrAnnotations.add(aa);
                        architectureAttrAnnotations.add(aa);
                        mappingsAttrAnnotations.add(aa);
                    }
                    break;
            }
        }

        for (Task t : specification.getApplication()) {
            for (AttributeAnnotation aa : applicationAttrAnnotations) {
                Matcher m = aa.getElemRegex().matcher(t.getId());
                if (m.find()) {
                    t.setAttribute(aa.getAttrName(), aa.getAttrValue());
                }
            }
        }

        for (Resource r : specification.getArchitecture()) {
            for (AttributeAnnotation aa : architectureAttrAnnotations) {
                Matcher m = aa.getElemRegex().matcher(r.getId());
                if (m.find()) {
                    r.setAttribute(aa.getAttrName(), aa.getAttrValue());
                }
            }
        }

        for (Mapping<Task, Resource> mp : specification.getMappings()) {
            for (AttributeAnnotation aa : mappingsAttrAnnotations) {
                Matcher m = aa.getElemRegex().matcher(mp.getId());
                if (m.find()) {
                    mp.setAttribute(aa.getAttrName(), aa.getAttrValue());
                }
            }
        }

        for (AttributeAnnotation aa : specificationAttrAnnotations) {
            specification.setAttribute(aa.getAttrName(), aa.getAttrValue());
        }
    }

    @Override
    public int getPriority() {
        return 0;
    }

}
